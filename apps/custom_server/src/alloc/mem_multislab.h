#ifndef SEL4_MISC_MEM_MULTISLAB_H
#define SEL4_MISC_MEM_MULTISLAB_H

// this is a special-purpose allocator for allocating chunks of memory when the chunks are generally expected to be from
// a limited (but unspecified) set of sizes.

#include "mem_arena_ao.h"
#include "mem_slab.h"

#define MULTISLAB_ALLOC_MULTIPLE MEM_SLAB_MIN_UNIT
#define MAX_ALLOWED_MULTISLAB_ALLOCATION MEM_SLAB_MAX_UNIT
#define MULTISLAB_SUBSLAB_COUNT (MAX_ALLOWED_MULTISLAB_ALLOCATION / MULTISLAB_ALLOC_MULTIPLE)

// TODO: optimize this further for memory
struct mem_multislab {
    struct mem_arena_ao arena;
    struct mem_slab subslabs[MULTISLAB_SUBSLAB_COUNT];
};

seL4_Error mem_multislab_create(struct mem_multislab *multislab);
void *mem_multislab_malloc(struct mem_multislab *multislab, size_t size);
void mem_multislab_free(struct mem_multislab *multislab, void *ptr, size_t size);
void mem_multislab_destroy(struct mem_multislab *multislab);

#endif //SEL4_MISC_MEM_MULTISLAB_H
