#ifndef SEL4_MISC_MEM_FX_H
#define SEL4_MISC_MEM_FX_H

// this is an allocator that is designed for allocating small (i.e. at most half a page) fixed-size structures and
// freeing them, as repeatedly as necessary. it runs with a pretty fast O(1) amortized, I think, but I haven't done the
// analysis. It gets this speed by not reclaiming memory allocated towards entities of one size - once a chunk of memory
// has been allocated for a 12-byte use, for example, those specific 12 bytes will always be a 12-byte chunk of memory.
// this is not suitable for all use-cases, but should be suitable for a number of common ones.

#include "../basic.h"

#define MEM_FX_DECL(t,x) t x = mem_fx_alloc(sizeof(t))
#define MEM_FX_ALLOC(x) x = mem_fx_alloc(sizeof(*x))
#define MEM_FX_FREE(x) mem_fx_free(x, sizeof(*x))

seL4_Error mem_fx_init(void);
void *mem_fx_alloc(size_t size);
void mem_fx_free(void *data, size_t size);

#endif //SEL4_MISC_MEM_FX_H