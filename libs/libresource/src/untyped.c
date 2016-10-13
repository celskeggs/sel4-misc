#include <resource/untyped.h>
#include <resource/cslot.h>
#include <resource/mem_fx.h>

typedef void *untyped_4m_ref;

// tracks two sizes:
// 4 MIB (size of large pages)
// 4 KIB (size of small pages, size of various miscellaneous structures)

struct s_4m {
    seL4_Untyped ut;
    seL4_CPtr aux; // no defined use here. TODO optimize memory use
    struct s_4m *next;
};

struct s_4k {
    seL4_Untyped ut;
    seL4_CPtr aux; // no defined use here. TODO optimize memory use
    struct s_4k *next;
};

#define CACHE_4K_COUNT 4 // todo: decide on this empirically
static struct s_4m *avail_4ms = NULL;
static struct s_4k *avail_4ks = NULL;
static uintptr_t avail_4ks_count = 0; // reflects the length of avail_4ks

// =================== 4M CHUNKS ===================

static bool untyped_add_memory_4m(seL4_Untyped ut) {
    seL4_CPtr aux = cslot_alloc();
    if (aux == seL4_CapNull) {
        ERRX_TRACEPOINT;
        return false;
    }
    struct s_4m *node = mem_fx_alloc(sizeof(struct s_4m));
    if (node == NULL) {
        cslot_free(aux);
        ERRX_TRACEPOINT;
        return false;
    }
    node->ut = ut;
    node->aux = aux;
    node->next = avail_4ms;
    avail_4ms = node;
    return true;
}

untyped_4m_ref untyped_allocate_4m(void) {
    if (avail_4ms == NULL) {
        ERRX_RAISE_GENERIC(GERR_MEMORY_POOL_EXHAUSTED);
        return NULL;
    } else {
        struct s_4m *found = avail_4ms;
        avail_4ms = found->next;
        found->next = NULL;
        return found;
    }
}

seL4_Untyped untyped_ptr_4m(untyped_4m_ref mem) {
    assert(mem != NULL);
    return ((struct s_4m *) mem)->ut;
}

seL4_CPtr untyped_auxptr_4m(untyped_4m_ref mem) {
    assert(mem != NULL);
    return ((struct s_4m *) mem)->aux;
}

void untyped_free_4m(untyped_4m_ref mem) {
    struct s_4m *n = (struct s_4m *) mem;
    assert(n != NULL && n->next == NULL);
    n->next = avail_4ms;
    avail_4ms = n;
    assert(cslot_delete(n->aux));
    assert(cslot_revoke(n->ut));
}

// =================== 4K CHUNKS ===================

static bool untyped_add_memory_4k(seL4_Untyped ut) {
    seL4_CPtr aux = cslot_alloc();
    if (aux == seL4_CapNull) {
        ERRX_TRACEPOINT;
        return false;
    }
    struct s_4k *node = mem_fx_alloc(sizeof(struct s_4k));
    if (node == NULL) {
        cslot_free(aux);
        ERRX_TRACEPOINT;
        return false;
    }
    node->ut = ut;
    node->aux = aux;
    node->next = avail_4ks;
    avail_4ks = node;
    avail_4ks_count++;
    return true;
}

static bool refill_running = false;

untyped_4k_ref untyped_allocate_4k(void) {
    if (avail_4ks_count <= CACHE_4K_COUNT && !refill_running && !mem_fx_is_allocating()) {
        assert((avail_4ks_count == 0) == (avail_4ks == NULL));
        // we're running out of memory - refill from 4M pool, assuming that this isn't a recursion while we do that
        refill_running = true;

        untyped_4m_ref larger = untyped_allocate_4m(); // TODO: have some way to rejoin large blocks? maybe?
        if (larger == NULL) {
            refill_running = false;
            ERRX_TRACEPOINT;
            return NULL;
        }
        seL4_Untyped larger_ut = untyped_ptr_4m(larger);

        assert(larger_ut != seL4_CapNull);
        for (uint32_t i = 0; i < 1U << (BITS_4MIB - BITS_4KIB); i++) {
            seL4_CPtr slot = cslot_alloc();
            if (slot == seL4_CapNull) {
                ERRX_TRACEPOINT;
                return false;
            }
            if (!cslot_retype(larger_ut, seL4_UntypedObject, i << BITS_4KIB, BITS_4KIB, slot, 1)) {
                ERRX_TRACEPOINT;
                return false; // TODO: better cleanup (?)
            }
            // NOTE: might recurse back to this allocator!
            if (!untyped_add_memory_4k(slot)) {
                ERRX_TRACEPOINT;
                return false;
            }
        }
        refill_running = false;
        // go and allocate normally now
    }
    if (avail_4ks == NULL) {
        assert(avail_4ks_count == 0);
        ERRX_RAISE_GENERIC(GERR_MEMORY_POOL_EXHAUSTED);
        return NULL;
    } else {
        assert(avail_4ks_count > 0);
        struct s_4k *found = avail_4ks;
        avail_4ks = found->next;
        found->next = NULL;
        return found;
    }
}

seL4_Untyped untyped_ptr_4k(untyped_4k_ref mem) {
    assert(mem != NULL);
    return ((struct s_4k *) mem)->ut;
}

seL4_CPtr untyped_auxptr_4k(untyped_4k_ref mem) {
    assert(mem != NULL);
    return ((struct s_4k *) mem)->aux;
}

void untyped_free_4k(untyped_4k_ref mem) {
    struct s_4k *n = (struct s_4k *) mem;
    assert(n != NULL && n->next == NULL);
    n->next = avail_4ks;
    avail_4ks = n;
    assert(cslot_delete(n->aux));
    assert(cslot_revoke(n->ut));
}

// =================== MEMORY ADDITION ===================

// if this fails, data structures may be corrupted. see below for reasons.
bool untyped_add_memory(seL4_Untyped ut, int size_bits) {
    if (size_bits > BITS_4MIB) {
        assert(ut != seL4_CapNull);
        for (uint32_t i = 0; i < 1U << (size_bits - BITS_4MIB); i++) {
            seL4_CPtr slot = cslot_alloc();
            if (slot == seL4_CapNull) {
                ERRX_TRACEPOINT;
                return false;
            }
            if (!cslot_retype(ut, seL4_UntypedObject, i << BITS_4MIB, BITS_4MIB, slot, 1)) {
                ERRX_TRACEPOINT;
                return false; // TODO: better cleanup (?)
            }
            if (!untyped_add_memory_4m(slot)) {
                ERRX_TRACEPOINT;
                return false;
            }
        }
        return true;
    } else if (size_bits == BITS_4MIB) {
        return untyped_add_memory_4m(ut);
    } else if (size_bits > BITS_4KIB) {
        assert(ut != seL4_CapNull);
        for (uint32_t i = 0; i < 1U << (size_bits - BITS_4KIB); i++) {
            seL4_CPtr slot = cslot_alloc();
            if (slot == seL4_CapNull) {
                ERRX_TRACEPOINT;
                return false;
            }
            if (!cslot_retype(ut, seL4_UntypedObject, i << BITS_4KIB, BITS_4KIB, slot, 1)) {
                ERRX_TRACEPOINT;
                return false; // TODO: better cleanup (?)
            }
            if (!untyped_add_memory_4k(slot)) {
                ERRX_TRACEPOINT;
                return false;
            }
        }
        return true;
    } else if (size_bits == BITS_4KIB) {
        return untyped_add_memory_4k(ut);
    } else {
        assert(size_bits > 0 && size_bits < BITS_4KIB);
        // note: we're wasting these, but it's not very much memory in the big scheme of things
        return true;
    }
}

bool untyped_add_boot_memory(seL4_BootInfo *info) {
    seL4_Word len = info->untyped.end - info->untyped.start;
    for (seL4_Word i = 0; i < len; i++) {
        if (!untyped_add_memory(info->untyped.start + i, info->untypedSizeBitsList[i])) {
            ERRX_TRACEPOINT;
            return false;
        }
    }
    return true;
}
