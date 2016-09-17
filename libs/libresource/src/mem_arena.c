#include <resource/mem_arena.h>

seL4_Error mem_arena_allocate(struct mem_arena *arena, size_t approximate_size) {
    size_t real_size = mem_vspace_alloc_slice(&arena->space, approximate_size);
    if (real_size == 0) {
        arena->user_root = NULL;
        return seL4_NotEnoughMemory;
    }
    struct mem_page_cookie root_cookie;
    void *root = mem_vspace_ptr(&arena->space);
    seL4_Error err = mem_page_map(root, &root_cookie);
    if (err != seL4_NoError) {
        mem_vspace_dealloc_slice(&arena->space);
        arena->user_root = NULL;
        return err;
    }
    size_t cookie_count = real_size / PAGE_SIZE;

    struct mem_page_cookie *cookies = (struct mem_page_cookie *) root;
    arena->user_root = &cookies[cookie_count];
    cookies[0] = root_cookie;
    for (size_t i = 1; i < cookie_count; i++) {
        err = mem_page_map(root + i * PAGE_SIZE, &cookies[i]);
        if (err != seL4_NoError) {
            // deallocate all of the pages
            while (i-- > 0) {
                mem_page_free(&cookies[i]);
            }
            mem_vspace_dealloc_slice(&arena->space);
            arena->user_root = NULL;
            return err;
        }
    }
    return seL4_NoError;
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
