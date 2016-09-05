#include "mem_fxalloc.h"

static seL4_Error setup_vspace(struct mem_fxalloc *fxalloc, struct mem_fxalloc_space_node *prealloc, size_t size) {
    struct mem_fxalloc_space_node *space_node = prealloc;
    struct mem_fxalloc_page_node *page_node = &prealloc->page_root;

    size_t actual = mem_vspace_alloc_slice(&space_node->vspace, size);
    if (actual == 0) {
        return seL4_NotEnoughMemory;
    }
    seL4_Error err = mem_page_map(mem_vspace_ptr(&space_node->vspace), &page_node->cookie);
    if (err != seL4_NoError) {
        mem_vspace_dealloc_slice(&space_node->vspace);
        return err;
    }

    fxalloc->current_ptr = mem_vspace_ptr(&space_node->vspace);
    fxalloc->alloc_ptr = fxalloc->current_ptr + PAGE_SIZE;
    fxalloc->end_ptr = fxalloc->current_ptr + mem_vspace_size(&space_node->vspace);

    // preallocate a small chunk of memory for the next vspace creation
    fxalloc->space_cached_node = (struct mem_fxalloc_space_node *) fxalloc->current_ptr;
    fxalloc->current_ptr = (void *) (fxalloc->space_cached_node + 1);

    space_node->next = fxalloc->space_head;
    fxalloc->space_head = space_node;
    page_node->next = fxalloc->page_head;
    fxalloc->page_head = page_node;
    return seL4_NoError;
}

seL4_Error mem_fxalloc_create(struct mem_fxalloc *fxalloc) {
    fxalloc->space_head = NULL;
    fxalloc->page_head = NULL;
    return setup_vspace(fxalloc, &fxalloc->space_root, PAGE_SIZE * 16);
}

static inline void validate_allocation_size(size_t size) {
    assert(MIN_FXALLOC_UNIT <= size && size <= MAX_FXALLOC_UNIT);
    assert((size & (MIN_FXALLOC_UNIT - 1)) == 0);
}

// this preserves a valid setup of the data structure
static seL4_Error acquire_another_page(struct mem_fxalloc *fxalloc) {
    if (fxalloc->alloc_ptr >= fxalloc->end_ptr) {
        // acquire new vspace and page
        size_t goal = mem_vspace_size(&fxalloc->space_head->vspace) * 2; // goal: a vspace twice as big as the last
        return setup_vspace(fxalloc, fxalloc->space_cached_node, goal);
        // we might throw away a bit of space at the end of our old vspace here, but it's at most
        // (MAX_FXALLOC_UNIT - MIN_FXALLOC_UNIT), which is something like 2044 bytes worst case scenario.
        // that's okay in terms of overhead.
    }
    // allocate a new page and use a bit of memory (much less than we just allocated) to store its cookie
    struct mem_page_cookie cookie;
    seL4_Error err = mem_page_map(fxalloc->alloc_ptr, &cookie);
    if (err != seL4_NoError) {
        return err;
    }
    fxalloc->alloc_ptr += PAGE_SIZE;
    assert(fxalloc->alloc_ptr <= fxalloc->end_ptr);
    struct mem_fxalloc_page_node *new_node = (struct mem_fxalloc_page_node *) fxalloc->current_ptr;
    fxalloc->current_ptr = (void *) (new_node + 1);
    new_node->cookie = cookie;
    new_node->next = fxalloc->page_head;
    fxalloc->page_head = new_node;
    // and we're back to a perfectly valid state
    return seL4_NoError;
}

void *mem_fxalloc_alloc(struct mem_fxalloc *fxalloc, size_t size) {
    validate_allocation_size(size);

    assert(fxalloc->current_ptr != NULL);
    while (fxalloc->current_ptr + size > fxalloc->alloc_ptr) {
        seL4_Error err = acquire_another_page(fxalloc);
        if (err != seL4_NoError) {
            return NULL;
        }
    }

    // finally, provide the final chunk to the user.
    void *out = fxalloc->current_ptr;
    fxalloc->current_ptr += size;
    return out;
}

void mem_fxalloc_destroy(struct mem_fxalloc *fxalloc) {
    // we have a hard thing to do here: we need to deallocate all of the vspaces and pages, but the pages are only
    // legitimized by the vspace's existence, and tracking info for the vspaces are in the pages.
    // we resolve this by ignoring the vspace's legitimacy-lending and just don't try to allocate any new vspace during
    // this method.

    // first, destroy all the vspaces.
    struct mem_fxalloc_space_node *node = fxalloc->space_head;
    while (node != NULL) {
        mem_vspace_dealloc_slice(&node->vspace);
        node = node->next;
    }

    // next, destroy all the pages. since we do this in reverse order, page metadata will be destroyed later than its
    // contents.
    struct mem_fxalloc_page_node *page = fxalloc->page_head;
    while (page != NULL) {
        mem_page_free(&page->cookie);
        page = page->next;
    }

    // avoid having any state that could mean anything if someone reuses this incorrectly
    fxalloc->current_ptr = fxalloc->alloc_ptr = fxalloc->end_ptr = NULL;
    fxalloc->page_head = NULL;
    fxalloc->space_head = NULL;
    fxalloc->space_cached_node = NULL;
}
