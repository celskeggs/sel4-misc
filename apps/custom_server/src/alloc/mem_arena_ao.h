#ifndef SEL4_MISC_MEM_ARENA_AO_H
#define SEL4_MISC_MEM_ARENA_AO_H

// a special-purpose allocator that allocates from an expanding memory arena, but does not allow individual freeing.

#include "mem_arena.h"

#define MIN_ARENA_AO_MUL_BITS 4
#define MIN_ARENA_AO_BITS (seL4_PageBits + MIN_ARENA_AO_MUL_BITS)
// 32 bits would be the entire memory space at once, but we wouldn't be able to allocate beyond one less than that.
#define MAX_ARENA_AO_BITS 31
#define MAX_ARENA_AO_COUNT (MAX_ARENA_AO_BITS - MIN_ARENA_AO_BITS + 1)
seL4_CompileTimeAssert(MAX_ARENA_AO_COUNT == 16);

struct mem_arena_ao {
    struct mem_arena arenas[MAX_ARENA_AO_COUNT];
    uint8_t current_arena;
    uintptr_t current_offset;
};

seL4_Error mem_arena_ao_create(struct mem_arena_ao *arena_ao);
void *mem_arena_ao_allocate_page(struct mem_arena_ao *arena_ao);
void mem_arena_ao_destroy(struct mem_arena_ao *arena_ao);

#endif //SEL4_MISC_MEM_ARENA_AO_H
