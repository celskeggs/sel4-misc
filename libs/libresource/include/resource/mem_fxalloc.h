#ifndef SEL4_MISC_MEM_FXALLOC_H
#define SEL4_MISC_MEM_FXALLOC_H

#include "mem_vspace.h"
#include "mem_page.h"

#define MIN_FXALLOC_UNIT (sizeof(void *))

// we need to save cookies for our allocated pages, so that they can be freed later. this linked list tracks those pages
struct mem_fxalloc_page_node {
    struct mem_page_cookie cookie;
    struct mem_fxalloc_page_node *next;
};

struct mem_fxalloc {
    struct mem_vspace vspace;

    // a pointer to the first byte of memory available to be used for a new chunk of any size.
    // aligned to MIN_FXALLOC_UNIT multiples.
    void *current_ptr;
    // a pointer to the first byte of memory that has reserved address space but doesn't have an allocated page.
    // aligned to page boundaries.
    void *alloc_ptr;
    // a pointer to the first byte of memory after the reserved address space section. aligned to page boundaries.
    void *end_ptr;

    // reserved memory space for the initial entry in the page linked list
    struct mem_fxalloc_page_node page_root;

    // head for the page linked list - the most recently allocated entry.
    struct mem_fxalloc_page_node *page_head;
};

seL4_Error mem_fxalloc_create(struct mem_fxalloc *fxalloc, size_t size_hint);

size_t mem_fxalloc_size(struct mem_fxalloc *fxalloc);

bool mem_fxalloc_is_full(struct mem_fxalloc *fxalloc);

void *mem_fxalloc_alloc(struct mem_fxalloc *fxalloc, size_t size);

void mem_fxalloc_destroy(struct mem_fxalloc *fxalloc);

#endif //SEL4_MISC_MEM_FXALLOC_H
