#ifndef SEL4_MISC_MEM_FXALLOC_H
#define SEL4_MISC_MEM_FXALLOC_H

// this is an allocator that is designed for allocating small (i.e. at most half a page) fixed-size structures and
// freeing them, as repeatedly as necessary. it runs with a pretty fast O(1) amortized, I think, but I haven't done the
// analysis. It gets this speed by not reclaiming memory allocated towards entities of one size - once a chunk of memory
// has been allocated for a 12-byte use, for example, those specific 12 bytes will always be a 12-byte chunk of memory.
// this is not suitable for all use-cases, but should be suitable for a number of common ones.

#include "mem_vspace.h"
#include "mem_page.h"

#define MIN_FXALLOC_UNIT (sizeof(void *))
#define MAX_FXALLOC_UNIT (PAGE_SIZE / 2)

// we need to save cookies for our allocated pages, so that they can be freed later. this linked list tracks those pages
struct mem_fxalloc_page_node {
    struct mem_page_cookie cookie;
    struct mem_fxalloc_page_node *next;
};

// multiple address spaces can be allocated and are tracked together
struct mem_fxalloc_space_node {
    struct mem_vspace vspace;
    struct mem_fxalloc_space_node *next;

    // contains preallocated memory for storing the cookie for the first page in this vspace
    struct mem_fxalloc_page_node page_root;
};

struct mem_fxalloc {
    // a pointer to the first byte of memory available to be used for a new chunk of any size.
    // aligned to MIN_FXALLOC_UNIT multiples.
    void *current_ptr;
    // a pointer to the first byte of memory that has reserved address space but doesn't have an allocated page.
    // aligned to page boundaries.
    void *alloc_ptr;
    // a pointer to the first byte of memory after the reserved address space section. aligned to page boundaries.
    void *end_ptr;

    // reserved memory space for the initial entry in the vspace linked list
    struct mem_fxalloc_space_node space_root;

    // preallocated memory space for the next vspace tracking node
    struct mem_fxalloc_space_node *space_cached_node;

    // heads for the space and page linked lists - these are the most recently allocated entries.
    struct mem_fxalloc_space_node *space_head;
    struct mem_fxalloc_page_node *page_head;
};

seL4_Error mem_fxalloc_create(struct mem_fxalloc *fxalloc);

void *mem_fxalloc_alloc(struct mem_fxalloc *fxalloc, size_t size);

void mem_fxalloc_destroy(struct mem_fxalloc *fxalloc);

#endif //SEL4_MISC_MEM_FXALLOC_H
