#include <resource/mem_fx.h>
#include <resource/mem_fxlarge.h>
#include <resource/mem_vspace.h>
#include <resource/mem_page.h>

#define COOKIE_COUNT 168
struct vspace_zone {
    struct mem_vspace zone;
    struct mem_page_cookie cookies[COOKIE_COUNT];
    uint32_t avail_count;
    struct vspace_zone *next;
};
seL4_CompileTimeAssert(sizeof(struct vspace_zone) <= 2048);

static uint32_t total_avail = 0;
static struct vspace_zone *first_zone = NULL;

static bool expand_large_pool() {
    struct vspace_zone *new_zone = mem_fx_alloc(sizeof(struct vspace_zone));
    if (new_zone == NULL) {
        ERRX_TRACEPOINT;
        return false;
    }
    size_t size = mem_vspace_alloc_slice_spec(&new_zone->zone, PAGE_SIZE * COOKIE_COUNT);
    if (size == 0) {
        mem_fx_free(new_zone, sizeof(struct vspace_zone));
        ERRX_TRACEPOINT;
        return false;
    }
    assert(size == PAGE_SIZE * COOKIE_COUNT);
    new_zone->avail_count = COOKIE_COUNT;
    total_avail += COOKIE_COUNT;
    memset(new_zone->cookies, 0, sizeof(new_zone->cookies));
    new_zone->next = first_zone;
    first_zone = new_zone;
    return true;
}

static struct vspace_zone *get_first_with_avail(void) {
    if (total_avail <= 0) {
        if (!expand_large_pool()) {
            ERRX_TRACEPOINT;
            return NULL;
        }
        assert(total_avail > 0);
    }
    assert(first_zone != NULL); // should be allocated by expand_large_pool above...
    struct vspace_zone *cur = first_zone;
    while (cur->avail_count == 0) {
        cur = cur->next;
        assert(cur != NULL); // should never run out, since total_avail > 0
    }
    return cur;
}

static struct vspace_zone *get_container(void *ptr) {
    struct vspace_zone *cur = first_zone;
    while (true) {
        // should never happen, since in theory the pointer was allocated from us. we don't have any way to signal error, anyway.
        assert(cur != NULL);

        void *zptr = mem_vspace_ptr(&cur->zone);
        if (ptr >= zptr && ptr < zptr + mem_vspace_size(&cur->zone)) {
            return cur;
        }
        cur = cur->next;
    }
}

void *mem_fxlarge_alloc(void) {
    struct vspace_zone *cur = get_first_with_avail();
    if (cur == NULL) {
        ERRX_TRACEPOINT;
        return NULL;
    }
    assert(cur->avail_count > 0);
    // search for first free
    uint32_t i = 0;
    while (mem_page_valid(&cur->cookies[i])) {
        i++;
        assert(i < COOKIE_COUNT); // because avail_count > 0, this should never happen.
    }
    void *goalptr = mem_vspace_ptr(&cur->zone) + PAGE_SIZE * i;
    assert((((uintptr_t) goalptr) & (PAGE_SIZE - 1)) == 0);
    if (!mem_page_map(goalptr, &cur->cookies[i])) {
        ERRX_TRACEPOINT;
        return NULL;
    }
    cur->avail_count--;
    total_avail--;
    return goalptr;
}

void mem_fxlarge_free(void *data) {
    assert((((uintptr_t) data) & (PAGE_SIZE - 1)) == 0);
    struct vspace_zone *cur = get_container(data);
    uint32_t index = (data - mem_vspace_ptr(&cur->zone)) / PAGE_SIZE;
    assert(index < COOKIE_COUNT);
    assert(mem_page_valid(&cur->cookies[index]));
    mem_page_free(&cur->cookies[index]);
    cur->avail_count++;
    total_avail++;
}
