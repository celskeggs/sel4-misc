#include <resource/mem_fx.h>
#include <resource/cslot.h>
#include "small.h"
#include "untyped.h"

#define SMALL_TABLE_ALLOC_BITS 12
#define SMALL_TABLE_BITS (SMALL_TABLE_ALLOC_BITS - 4) // 2^4 = 16, the size of one entry
#define SMALL_TABLE_SIZE (BIT(SMALL_TABLE_BITS))
#define SMALL_TABLE_BITMAP_LEN (SMALL_TABLE_SIZE / 64) // number of uint64_t values needed for the bitmap

struct small_table { // 4K: 256 allocatable 16-byte chunks
    uint16_t free;
    uint8_t bitmap_free_index;
    uint64_t bitmap[SMALL_TABLE_BITMAP_LEN]; // 1 if free; 0 if allocated
    seL4_CPtr chunk_base; // the first CSlot of the chunk; +1023 for last CSlot of the chunk
    untyped_4k_ref ref;
    struct small_table *prev;
    struct small_table *next;
};
static uint64_t total_free = 0;
static struct small_table *cached_free = NULL; // pointer to the last-known free location in the circular linked list

static struct small_table *alloc_small_table() {
    seL4_CompileTimeAssert(SMALL_TABLE_ALLOC_BITS == BITS_4KIB);
    untyped_4k_ref ref = untyped_allocate_4k();
    if (ref == NULL) {
        ERRX_TRACEPOINT;
        return NULL;
    }
    struct small_table *tab = mem_fx_alloc(sizeof(struct small_table));
    if (tab == NULL) {
        untyped_free_4k(ref);
        ERRX_TRACEPOINT;
        return NULL;
    }
    tab->ref = ref;
    tab->chunk_base = cslot_alloc_slab(SMALL_TABLE_SIZE);
    if (tab->chunk_base == seL4_CapNull) {
        mem_fx_free(tab, sizeof(struct small_table));
        untyped_free_4k(ref);
        ERRX_TRACEPOINT;
        return NULL;
    }
    tab->free = SMALL_TABLE_SIZE;
    total_free += SMALL_TABLE_SIZE;
    tab->bitmap_free_index = 0;
    assert(sizeof(tab->bitmap) == sizeof(uint64_t) * 4);
    memset(&tab->bitmap, 0xFF, sizeof(tab->bitmap));
    if (cached_free == NULL) { // no list; init it
        cached_free = tab->prev = tab->next = tab;
    } else { // insert ourself into the list
        tab->next = cached_free;
        tab->prev = cached_free->prev;
        tab->prev->next = tab;
        tab->next->prev = tab;
        cached_free = tab; // we're now the most likely table to have free slots
    }
    return tab;
}

static struct small_table *get_nonfull_small_table() {
    if (total_free == 0) {
        // nothing left; just allocate
        return alloc_small_table();
    }
    // start at a place that probably has something free, and go around the list until we find something.
    struct small_table *orig_cached = cached_free;
    while (cached_free->free == 0) {
        cached_free = cached_free->next;
        assert(cached_free != orig_cached); // should never happen: there was free space, remember?
    }
    return cached_free;
}

seL4_CPtr small_table_alloc(int type) { // 16 byte objects only
    struct small_table *nonfull_table = get_nonfull_small_table();
    if (nonfull_table == NULL) {
        ERRX_TRACEPOINT;
        return seL4_CapNull;
    }
    assert(nonfull_table->free > 0);
    while (nonfull_table->bitmap[nonfull_table->bitmap_free_index] == 0) {
        // fully allocated? try the next.
        nonfull_table->bitmap_free_index++;
        // if we reach the end, that means our table wasn't actually nonfull. oops!
        assert(nonfull_table->bitmap_free_index < SMALL_TABLE_BITMAP_LEN);
    }
    uint64_t bits = nonfull_table->bitmap[nonfull_table->bitmap_free_index];
    assert(bits != 0);
    // MSB (max) -> 0001011111000110111111 <- LSB (0)
    // question is: where's the first one?
    // in that example, at bit 0
    // in: 00000111111111111111110
    // it's bit 1
    // but in: 00001111100000000000000
    // the number's much higher.
    uint8_t first_alloc_index = __builtin_ctzll(bits);
    assert(bits & (1uLL << first_alloc_index));
    bits &= ~(1uLL << first_alloc_index); // turn off the first-available bit
    // we're ready to commit our changes, but hold off until we've actually retyped the memory
    uint32_t our_offset = nonfull_table->bitmap_free_index * 64uL + first_alloc_index;
    seL4_CPtr ptr = nonfull_table->chunk_base + our_offset;
    if (!cslot_retype(untyped_ptr_4k(nonfull_table->ref), type, 16 * our_offset, 0, ptr, 1)) {
        ERRX_TRACEPOINT;
        return seL4_CapNull;
    }
    // commit changes
    nonfull_table->bitmap[nonfull_table->bitmap_free_index] = bits;
    nonfull_table->free--;
    total_free--;
    return ptr;
}

void small_table_free(seL4_CPtr ptr) {
    assert(ptr != seL4_CapNull);
    struct small_table *ref = cached_free;
    while (ptr < ref->chunk_base || ptr >= ref->chunk_base + SMALL_TABLE_SIZE) {
        ref = ref->next;
        assert(ref != cached_free); // should never happen, unless it's not from us
    }
    assert(cslot_revoke(ptr)); // TODO: check that revoke works properly
    assert(cslot_delete(ptr));
    uint32_t offset = ptr - ref->chunk_base;
    assert(offset < SMALL_TABLE_SIZE);
    uint8_t bitmap_offset = (uint8_t) (offset >> 6);
    uint8_t bit_index = (uint8_t) (offset & 63);
    uint64_t bits = ref->bitmap[bitmap_offset];
    uint64_t mask = (1uLL << bit_index);
    assert((bits & mask) == 0);
    bits |= mask;
    ref->bitmap[bitmap_offset] = bits;
    ref->free++;
    total_free++;
    if (bitmap_offset < ref->bitmap_free_index) {
        ref->bitmap_free_index = bitmap_offset;
    }
    if (cached_free->free < ref->free) {
        cached_free = ref;
    }
}
