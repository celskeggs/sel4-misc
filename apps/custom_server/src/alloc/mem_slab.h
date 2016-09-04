#ifndef SEL4_MISC_MEM_SLAB_H
#define SEL4_MISC_MEM_SLAB_H

#include "mem_arena_ao.h"

struct mem_slab_header {
    struct mem_slab_header *next;
};

struct mem_slab {
    struct mem_slab_header *linked_start;
    void *remaining_start;
    void *remaining_end;
    size_t unit_size;
};

#define MEM_SLAB_MIN_UNIT (sizeof(struct mem_slab_header))
#define MEM_SLAB_MIN_BITS 2 // hardcoded and guarded by the below assert
seL4_CompileTimeAssert(MEM_SLAB_MIN_UNIT == (1 << MEM_SLAB_MIN_BITS));

#define MEM_SLAB_MAX_UNIT (PAGE_SIZE / 2) // so that there are at least two per page. otherwise, why bother?
#define MEM_SLAB_MAX_BITS (seL4_PageBits - 1)
seL4_CompileTimeAssert(MEM_SLAB_MAX_UNIT == (1 << MEM_SLAB_MAX_BITS));

// TODO: remove this if unnecessary
#define MEM_SLAB_RANGE_BITS (MEM_SLAB_MAX_BITS - MEM_SLAB_MIN_BITS + 1)
seL4_CompileTimeAssert(MEM_SLAB_RANGE_BITS == 10);

// unit_size must be at least MEM_SLAB_MIN_UNIT and a multiple of four. powers of two work best.
// unit_size must be at most MEM_SLAB_MAX_UNIT
seL4_Error mem_slab_create(struct mem_slab *slab, size_t unit_size);
static inline size_t mem_slab_unit_size(struct mem_slab *slab) {
    return slab->unit_size;
}
void *mem_slab_malloc(struct mem_slab *slab, struct mem_arena_ao *allocator);
void mem_slab_free(struct mem_slab *slab, void *object);
// destruction of a mem_slab is done by destroying the attached allocator

#endif //SEL4_MISC_MEM_SLAB_H
