#include <sel4/assert.h>
#include "mem_slab.h"
#include "../basic.h"
#include "mem_page.h"

struct slab_line {
    struct slab_line *next; // only valid when unallocated
    uint8_t _padding[SLAB_SIZE - sizeof(void *)];
};
seL4_CompileTimeAssert(sizeof(struct slab_line) == SLAB_SIZE);

struct slab_header {
    struct slab_header *next_slab;
    struct slab_line *free;
    uint8_t free_count;
    uint8_t _padding[SLAB_SIZE - sizeof(void *) * 2 - sizeof(uint8_t)];
};
seL4_CompileTimeAssert(sizeof(struct slab_header) == SLAB_SIZE);

#define SLAB_ENT_COUNT 255
struct slab_full {
    struct slab_header header;
    // NOTE: must expand 'slab_header.free_count' bitwidth if this changes.
    struct slab_line lines[SLAB_ENT_COUNT];
};
seL4_CompileTimeAssert(sizeof(struct slab_full) == PAGE_SIZE);

static uint64_t total_free = 0;
static struct slab_full *first_slab = NULL;

static void *slab_get_nonempty(void) {
    if (total_free <= 0) {
        // allocate a new one!
        struct slab_full *new_slab = mem_page_alloc();
        new_slab->header.next_slab = &first_slab->header;
        new_slab->header.free = &new_slab->lines[0];
        for (int i = 1; i < SLAB_ENT_COUNT; i++) {
            new_slab->lines[i - 1].next = &new_slab->lines[i];
        }
        new_slab->lines[SLAB_ENT_COUNT - 1].next = NULL;
        first_slab = new_slab;
        new_slab->header.free_count = SLAB_ENT_COUNT;
        total_free += SLAB_ENT_COUNT;
    }
    WORKING HERE
}

void *slab_allocate() {

}

void slab_free(void *slab) {

}
