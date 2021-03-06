#ifndef LIB_RESOURCE_MEM_VSPACE_H
#define LIB_RESOURCE_MEM_VSPACE_H

#include <bedrock/types.h>
#include <bedrock/errx.h> // this module uses errx

struct mem_vspace {
    // represents two regions:
    // [alloc_begin .. middle], which is allocated, and [middle .. next->alloc_begin], which is free.
    void *alloc_begin;
    void *middle;
    struct mem_vspace *prev;
    struct mem_vspace *next;
};

void mem_vspace_setup(size_t image_size);

// "approximate" means that the actual size can be as low as 1/2 of the value or as high as 2x the value
// allocated space will be in a page multiple
// the mem_vspace MUST remain allocated until "dealloc_slice" returns!
// DO NOT USE THIS VSPACE ALLOCATION API TO HANDLE INDIVIDUAL ALLOCATIONS. ONLY WORK WITH BLOCKS.
// result is actual allocated size
size_t mem_vspace_alloc_slice(struct mem_vspace *zone, size_t approximate_size);

// a less efficient version with a precise allocation size - well, when rounded to a page.
// DO NOT USE THIS VSPACE ALLOCATION API TO HANDLE INDIVIDUAL ALLOCATIONS. ONLY WORK WITH BLOCKS.
size_t mem_vspace_alloc_slice_spec(struct mem_vspace *zone, size_t precise_size);

static inline void *mem_vspace_ptr(struct mem_vspace *zone) {
    return zone->alloc_begin;
}

static inline size_t mem_vspace_size(struct mem_vspace *zone) {
    return zone->middle - zone->alloc_begin;
}

void mem_vspace_dealloc_slice(struct mem_vspace *zone);

#endif //LIB_RESOURCE_MEM_VSPACE_H
