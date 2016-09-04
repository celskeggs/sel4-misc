#include "mem_arena_ao.h"

seL4_Error mem_arena_ao_create(struct mem_arena_ao *arena_ao) {
    arena_ao->current_offset = 0;
    arena_ao->current_arena = 0;
    return mem_arena_allocate(&arena_ao->arenas[0], 1 << MIN_ARENA_AO_BITS);
}

void *mem_arena_ao_allocate_page(struct mem_arena_ao *arena_ao) {
    assert(arena_ao->current_arena < MAX_ARENA_AO_COUNT);
    size_t max_ptr = mem_arena_size(&arena_ao->arenas[arena_ao->current_arena]);
    if (arena_ao->current_offset >= max_ptr) {
        // needs another arena
        if (arena_ao->current_arena >= MAX_ARENA_AO_COUNT - 1) {
            // not possible - no more room for arenas to be allocated
            return NULL;
        }
        arena_ao->current_arena++;
        seL4_Error err = mem_arena_allocate(&arena_ao->arenas[arena_ao->current_arena],
                                            1U << (MIN_ARENA_AO_BITS + arena_ao->current_arena));
        if (err != seL4_NoError) {
            arena_ao->current_arena--;
            return NULL;
        }
        void *out = mem_arena_base(&arena_ao->arenas[arena_ao->current_arena]);
        arena_ao->current_offset = PAGE_SIZE;
        size_t offset = ((uintptr_t) out) & (PAGE_SIZE - 1);
        if (offset != 0) {
            out += PAGE_SIZE - offset;
        }
        return out;
    } else {
        // use existing memory
        void *out = mem_arena_base(&arena_ao->arenas[arena_ao->current_arena]) + arena_ao->current_offset;
        arena_ao->current_offset += PAGE_SIZE;
        size_t offset = ((uintptr_t) out) & (PAGE_SIZE - 1); // TODO: do this part better
        if (offset != 0) {
            out += PAGE_SIZE - offset;
        }
        return out;
    }
}

void mem_arena_ao_destroy(struct mem_arena_ao *arena_ao) {
    uint32_t i = arena_ao->current_arena + 1;
    while (i-- > 0) {
        mem_arena_deallocate(&arena_ao->arenas[i]);
    }
    arena_ao->current_arena = MAX_ARENA_AO_COUNT;
    arena_ao->current_offset = 0;
}
