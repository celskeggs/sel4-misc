#include <resource/mem_vspace.h>
#include <resource/mem_page.h>

// this will be our standard starting offset for allocating pages - directly after the first page table.
#define VSPACE_BEGIN_PTR ((void *) PAGE_TABLE_SIZE)
#define VSPACE_END_PTR ((void *) KERNEL_BASE_VADDR)
// 0x400000 - 0xDFFFFFFF

extern char __executable_start;
extern char _end;

// two circular linked lists

// VSPACE_END_PTR is the implied value for .alloc_begin of NULL (the final free zone end)
static inline void *get_end_ptr(struct mem_vspace *node) {
    return node->next == NULL ? VSPACE_END_PTR : node->next->alloc_begin;
}

static inline size_t get_free_size(struct mem_vspace *node) {
    return get_end_ptr(node) - node->middle;
}

static struct mem_vspace beta_zone;
static struct mem_vspace root_zone = {
        .alloc_begin = VSPACE_BEGIN_PTR,
        .middle = VSPACE_BEGIN_PTR,
        .next = &beta_zone,
        .prev = NULL,
};
static struct mem_vspace beta_zone = {
        .alloc_begin = NULL, // to be filled in
        .middle = NULL, // to be filled in
        .next = NULL,
        .prev = &root_zone,
};

void mem_vspace_setup(size_t image_size, void *ipc_buffer, void *boot_buffer) {
    // make sure we also don't allocate over the ipc buffer
    assert(&__executable_start + image_size == ipc_buffer);
    assert(&__executable_start + image_size + 0x1000 == boot_buffer);
    image_size += PAGE_SIZE;

    beta_zone.alloc_begin = (void *) ((uintptr_t) &__executable_start & ~(PAGE_TABLE_SIZE - 1));
    beta_zone.middle = (void *) (((uintptr_t) &__executable_start + image_size + PAGE_TABLE_SIZE - 1) &
                                 ~(PAGE_TABLE_SIZE - 1));
}

// from is an existing node with free space. to is an uninitialized node to build with that free space.
static void slice_entire_node(struct mem_vspace *from, struct mem_vspace *to) {
    assert(from->next->prev == from);
    assert(from->alloc_begin <= from->middle);
    assert(from->middle < from->next->alloc_begin);
    to->next = from->next;
    to->prev = from;
    to->alloc_begin = from->middle; // take the free space from the old node
    to->middle = from->next->alloc_begin; // no free space left
    from->next = to;
    to->next->prev = to;
    assert(to->prev->next == to);
}

// from is an existing node with free space. to is an uninitialized node to build with part of that free space.
static void slice_partial_node(struct mem_vspace *from, size_t goal_rounded, struct mem_vspace *to) {
    assert(from->next->prev == from);
    assert(from->alloc_begin <= from->middle);
    assert(from->middle < from->next->alloc_begin);
    to->next = from->next;
    to->prev = from;
    to->alloc_begin = from->middle; // take the free space from the old node
    to->middle = to->alloc_begin + goal_rounded; // take part of the free space, but also offer the remainder
    assert(to->middle < from->next->alloc_begin); // make sure we aren't taking more memory than is available
    from->next = to;
    to->next->prev = to;
    assert(to->prev->next == to);
}

// uses an O(n) algorithm - don't use this often or with small blocks
size_t mem_vspace_alloc_slice(struct mem_vspace *zone, size_t approximate_size) {
    size_t min_rounded = (approximate_size / 2 + 1 + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    size_t goal_rounded = (approximate_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    size_t max_rounded = (approximate_size * 2) & ~(PAGE_SIZE - 1);
    assert(min_rounded >= PAGE_SIZE);
    assert(goal_rounded >= min_rounded);
    assert(max_rounded >= goal_rounded);
    struct mem_vspace *cur = &root_zone;
    struct mem_vspace *smallest_block = NULL;
    size_t smallest_size = 0;
    // we should be at the beginning
    assert(cur->prev == NULL);
    // let's scan through for an empty block that we don't need to subdivide
    // but also find the smallest available block that's too big
    while (cur != NULL) {
        size_t size = get_free_size(cur);
        if (size >= min_rounded) {
            if (size <= max_rounded) {
                // good enough for us! use it.
                slice_entire_node(cur, zone);
                size_t real_size = mem_vspace_size(zone);
                assert(real_size == size);
                return real_size;
            } else {
                // too big - but we might need this if we have to subdivide
                if (smallest_block == NULL || size < smallest_size) {
                    smallest_block = cur;
                    smallest_size = size;
                }
            }
        }
        cur = cur->next;
    }
    if (smallest_block == NULL) {
        ERRX_RAISE_GENERIC(GERR_MEMORY_POOL_EXHAUSTED);
        return 0; // nothing is available with enough room
    }
    slice_partial_node(smallest_block, goal_rounded, zone);
    size_t real_size = mem_vspace_size(zone);
    assert(real_size == goal_rounded);
    return real_size;
}

void mem_vspace_dealloc_slice(struct mem_vspace *zone) {
    assert(zone->prev != NULL); // otherwise, somehow we're deallocating the root! I think...
    zone->prev->next = zone->next;
    if (zone->next != NULL) {
        zone->next->prev = zone->prev;
    }
    // by joining nodes, the entire space used by this node is implicitly marked as free as part of the last node's free
    // space (after 'middle' and before the next node's alloc_begin)
}
