#include <resource/mem_arena.h>

bool mem_arena_allocate(struct mem_arena *arena, size_t approximate_size) {
    size_t real_size = mem_vspace_alloc_slice(&arena->space, approximate_size);
    if (real_size == 0) {
        arena->user_root = NULL;
        return false;
    }
    struct mem_page_cookie root_cookie;
    void *root = mem_vspace_ptr(&arena->space);
    if (!mem_page_map(root, &root_cookie)) {
        mem_vspace_dealloc_slice(&arena->space);
        arena->user_root = NULL;
        return false;
    }
    size_t cookie_count = real_size / PAGE_SIZE;

    struct mem_page_cookie *cookies = (struct mem_page_cookie *) root;
    arena->user_root = &cookies[cookie_count];
    cookies[0] = root_cookie;
    for (size_t i = 1; i < cookie_count; i++) {
        if (!mem_page_map(root + i * PAGE_SIZE, &cookies[i])) {
            // deallocate all of the pages
            while (i-- > 0) {
                mem_page_free(&cookies[i]);
            }
            mem_vspace_dealloc_slice(&arena->space);
            arena->user_root = NULL;
            return false;
        }
    }
    return true;
}

void mem_arena_deallocate(struct mem_arena *arena) {
    struct mem_page_cookie *cookies = (struct mem_page_cookie *) mem_vspace_ptr(&arena->space);
    size_t cookie = mem_vspace_size(&arena->space) / PAGE_SIZE;
    while (cookie-- > 0) {
        mem_page_free(&cookies[cookie]);
    }
    mem_vspace_dealloc_slice(&arena->space);
    arena->user_root = NULL;
}
