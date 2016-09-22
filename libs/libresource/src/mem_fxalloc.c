#include <resource/mem_fxalloc.h>

bool mem_fxalloc_create(struct mem_fxalloc *fxalloc, size_t size_hint) {
    struct mem_fxalloc_page_node *page_node = &fxalloc->page_root;

    size_t actual = mem_vspace_alloc_slice(&fxalloc->vspace, size_hint);
    if (actual == 0) {
        return false;
    }
    if (!mem_page_map(mem_vspace_ptr(&fxalloc->vspace), &page_node->cookie)) {
        mem_vspace_dealloc_slice(&fxalloc->vspace);
        return false;
    }

    fxalloc->current_ptr = mem_vspace_ptr(&fxalloc->vspace);
    fxalloc->alloc_ptr = fxalloc->current_ptr + PAGE_SIZE;
    fxalloc->end_ptr = fxalloc->current_ptr + mem_vspace_size(&fxalloc->vspace);

    page_node->next = NULL;
    fxalloc->page_head = page_node;
    return true;
}

size_t mem_fxalloc_size(struct mem_fxalloc *fxalloc) {
    return mem_vspace_size(&fxalloc->vspace);
}

bool mem_fxalloc_is_full(struct mem_fxalloc *fxalloc) {
    return fxalloc->alloc_ptr >= fxalloc->end_ptr;
}

// this preserves a valid setup of the data structure
static bool acquire_another_page(struct mem_fxalloc *fxalloc) {
    if (mem_fxalloc_is_full(fxalloc)) {
        // no more room!
        ERRX_RAISE_SEL4(GERR_MEMORY_POOL_EXHAUSTED);
        return false;
    }
    // allocate a new page and use a bit of memory (much less than we just allocated) to store its cookie
    struct mem_page_cookie cookie;
    if (!mem_page_map(fxalloc->alloc_ptr, &cookie)) {
        return false;
    }
    fxalloc->alloc_ptr += PAGE_SIZE;
    assert(fxalloc->alloc_ptr <= fxalloc->end_ptr);
    struct mem_fxalloc_page_node *new_node = (struct mem_fxalloc_page_node *) fxalloc->current_ptr;
    fxalloc->current_ptr = (void *) (new_node + 1);
    new_node->cookie = cookie;
    new_node->next = fxalloc->page_head;
    fxalloc->page_head = new_node;
    // and we're back to a perfectly valid state
    return true;
}

void *mem_fxalloc_alloc(struct mem_fxalloc *fxalloc, size_t size) {
    assert(MIN_FXALLOC_UNIT <= size);
    assert((size & (MIN_FXALLOC_UNIT - 1)) == 0);

    assert(fxalloc->current_ptr != NULL);
    while (fxalloc->current_ptr + size > fxalloc->alloc_ptr) {
        if (fxalloc->current_ptr + size > fxalloc->end_ptr) {
            ERRX_RAISE_GENERIC(GERR_MEMORY_POOL_EXHAUSTED);
            return NULL; // no amount of page allocation will make this possible at this point
        }
        if (!acquire_another_page(fxalloc)) {
            return NULL;
        }
    }

    // finally, provide the next chunk to the user.
    void *out = fxalloc->current_ptr;
    fxalloc->current_ptr += size;
    return out;
}

void mem_fxalloc_destroy(struct mem_fxalloc *fxalloc) {
    // destroy all the pages. since we do this in reverse, page metadata will be destroyed later than page content.
    struct mem_fxalloc_page_node *page = fxalloc->page_head;
    while (page != NULL) {
        mem_page_free(&page->cookie);
        page = page->next;
    }

    mem_vspace_dealloc_slice(&fxalloc->vspace);

    // avoid having any state that could mean anything if someone reuses this incorrectly
    fxalloc->current_ptr = fxalloc->alloc_ptr = fxalloc->end_ptr = NULL;
    fxalloc->page_head = NULL;
}
