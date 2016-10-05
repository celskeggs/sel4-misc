#include <resource/mem_fx.h>
#include <resource/mem_fxlarge.h>
#include <resource/mem_vspace.h>
#include <resource/mem_page.h>

#define COOKIE_COUNT 168U
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

static inline bool is_avail(struct vspace_zone *zone, uint8_t page_count, uint32_t start) {
    assert(page_count >= MEM_FXLARGE_MIN_PAGES && page_count <= MEM_FXLARGE_MAX_PAGES);
    for (uint32_t i = 0; i < page_count; i++) {
        assert(start + i < COOKIE_COUNT);
        if (mem_page_valid(&zone->cookies[start + i])) {
            return false; // already used
        }
    }
    return true;
}

static inline bool has_any_avail(struct vspace_zone *zone, uint8_t page_count) {
    assert(page_count >= MEM_FXLARGE_MIN_PAGES && page_count <= MEM_FXLARGE_MAX_PAGES);
    for (uint32_t i = 0; i <= COOKIE_COUNT - page_count; i++) {
        if (is_avail(zone, page_count, i)) {
            return true;
        }
    }
    return false;
}

static struct vspace_zone *get_first_with_avail(uint8_t page_count) {
    assert(page_count >= MEM_FXLARGE_MIN_PAGES && page_count <= MEM_FXLARGE_MAX_PAGES);
    if (total_avail < page_count) {
        need_to_expand:
        if (!expand_large_pool()) {
            ERRX_TRACEPOINT;
            return NULL;
        }
        assert(total_avail >= page_count);
    }
    assert(first_zone != NULL); // should be allocated by expand_large_pool above...
    struct vspace_zone *cur = first_zone;
    while (cur->avail_count < page_count || (page_count > 1 && !has_any_avail(cur, page_count))) {
        cur = cur->next;
        if (cur == NULL) {
            assert(page_count > 1); // otherwise, this really shouldn't have happened, because there were some pages
            goto need_to_expand; // oops - we didn't catch this earlier. do it now, though.
        }
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

static inline bool map_range(struct vspace_zone *zone, void *ptr, uint8_t page_count, uint32_t start) {
    assert(page_count >= MEM_FXLARGE_MIN_PAGES && page_count <= MEM_FXLARGE_MAX_PAGES);
    for (uint32_t i = 0; i < page_count; i++) {
        if (!mem_page_map(ptr + PAGE_SIZE * i, &zone->cookies[start + i])) {
            while (i-- > 0) {
                mem_page_free(&zone->cookies[start + i]);
            }
            ERRX_TRACEPOINT;
            return false;
        }
    }
    return true;
}

void *mem_fxlarge_alloc(uint8_t page_count) {
    assert(page_count >= MEM_FXLARGE_MIN_PAGES && page_count <= MEM_FXLARGE_MAX_PAGES);
    struct vspace_zone *cur = get_first_with_avail(page_count);
    if (cur == NULL) {
        ERRX_TRACEPOINT;
        return NULL;
    }
    assert(cur->avail_count > 0);
    // search for first free
    uint32_t i = 0;
    while (!is_avail(cur, page_count, i)) {
        i++;
        assert(i <= COOKIE_COUNT - page_count); // because avail_count > 0, this should never happen.
    }
    void *goalptr = mem_vspace_ptr(&cur->zone) + PAGE_SIZE * i;
    assert((((uintptr_t) goalptr) & (PAGE_SIZE - 1)) == 0);
    if (!map_range(cur, goalptr, page_count, i)) {
        ERRX_TRACEPOINT;
        return NULL;
    }
    cur->avail_count -= page_count;
    total_avail -= total_avail;
    return goalptr;
}

void mem_fxlarge_free(void *data, uint8_t page_count) {
    assert(page_count >= MEM_FXLARGE_MIN_PAGES && page_count <= MEM_FXLARGE_MAX_PAGES);
    assert((((uintptr_t) data) & (PAGE_SIZE - 1)) == 0);
    struct vspace_zone *cur = get_container(data);
    uint32_t index = (data - mem_vspace_ptr(&cur->zone)) / PAGE_SIZE;
    assert(index < COOKIE_COUNT);
    assert(index + page_count <= COOKIE_COUNT);
    for (uint32_t i = 0; i < page_count; i++) {
        assert(mem_page_valid(&cur->cookies[index + i]));
        mem_page_free(&cur->cookies[index + i]);
    }
    cur->avail_count += page_count;
    total_avail += page_count;
}
