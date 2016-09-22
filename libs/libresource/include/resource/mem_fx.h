#ifndef SEL4_MISC_MEM_FX_H
#define SEL4_MISC_MEM_FX_H

#include <bedrock/bedrock.h>
#include <bedrock/errx.h> // this module uses errx

// this is an allocator that is designed for allocating small (i.e. at most half a page) fixed-size structures and
// freeing them, as repeatedly as necessary. it runs with a pretty fast O(1) amortized, I think, but I haven't done the
// analysis. It gets this speed by not reclaiming memory allocated towards entities of one size - once a chunk of memory
// has been allocated for a 12-byte use, for example, those specific 12 bytes will always be a 12-byte chunk of memory.
// this is not suitable for all use-cases, but should be suitable for a number of common ones.

bool mem_fx_init(void);
void *mem_fx_alloc(size_t size);
void mem_fx_free(void *data, size_t size);

// used by untyped.c to know when it should avoid calling allocations (to avoid recursion)
bool mem_fx_is_allocating(void);

#endif //SEL4_MISC_MEM_FX_H
