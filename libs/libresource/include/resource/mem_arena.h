#ifndef SEL4_MISC_MEM_ARENA_H
#define SEL4_MISC_MEM_ARENA_H

#include <bedrock/errx.h> // this module uses errx
#include "mem_vspace.h"
#include "mem_page.h"

struct mem_arena {
    void *user_root;
    struct mem_vspace space;
};

bool mem_arena_allocate(struct mem_arena *arena, size_t approximate_size);
static inline void *mem_arena_base(struct mem_arena *arena) {
    return arena->user_root;
}
static inline size_t mem_arena_size(struct mem_arena *arena) {
    return mem_vspace_size(&arena->space) + mem_vspace_ptr(&arena->space) - arena->user_root;
}
void mem_arena_deallocate(struct mem_arena *arena);

#endif //SEL4_MISC_MEM_ARENA_H
