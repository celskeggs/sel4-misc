#include "mem_slab.h"

seL4_Error mem_slab_create(struct mem_slab *slab, size_t unit_size) {
    assert(unit_size >= MEM_SLAB_MIN_UNIT);
    assert((unit_size & 3) == 0);
    assert(unit_size <= MEM_SLAB_MAX_UNIT);
    slab->unit_size = unit_size;
    slab->remaining_start = NULL;
    slab->remaining_end = NULL;
    slab->linked_start = NULL;
    return seL4_NoError;
}

void *mem_slab_malloc(struct mem_slab *slab, struct mem_arena_ao *allocator) {
    if (slab->linked_start != NULL) { // first try the freelist
        void *out = slab->linked_start;
        slab->linked_start = slab->linked_start->next;
        return out;
    } else {
        if (slab->remaining_start + slab->unit_size > slab->remaining_end) { // includes case where remaining_* == NULL
            // we don't have enough memory left to slab anything further
            slab->remaining_start = mem_arena_ao_allocate_page(allocator);
            if (slab->remaining_start == NULL) {
                return NULL;
            }
            slab->remaining_end = slab->remaining_start + PAGE_SIZE;
        }
        // now slab more memory
        // note: with slab units that are not a power of 2, it is possible for there to be extra unused space at the end
        void *out = slab->remaining_start;
        slab->remaining_start += slab->unit_size;
        return out;
    }
}

void mem_slab_free(struct mem_slab *slab, void *object) {
    assert(object != NULL);
    // TODO: make sure object is valid and in bounds? hmm
    struct mem_slab_header *obj = (struct mem_slab_header *) object;
    obj->next = slab->linked_start;
    slab->linked_start = obj;
}
