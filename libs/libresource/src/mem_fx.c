#include <resource/mem_fx.h>
#include <resource/mem_fxcache.h>
#include <resource/mem_fxseq.h>
#include <resource/mem_fxlarge.h>

static struct mem_fxcache fx_cache = MEM_FXCACHE_INIT;
static struct mem_fxseq fx_seq = MEM_FXSEQ_PREINIT;
static bool is_allocating;

bool mem_fx_init(void) {
    return mem_fxseq_init(&fx_seq);
}

bool mem_fx_is_allocating(void) {
    return is_allocating;
}

static inline size_t round_size(size_t size) {
    assert(size > 0 && size <= FXCACHE_MAX);
    if (size < FXCACHE_UNIT) {
        return FXCACHE_UNIT;
    }
    size_t rel = size & (FXCACHE_UNIT - 1);
    if (rel != 0) {
        return size + FXCACHE_UNIT - rel;
    } else {
        return size;
    }
}

void *mem_fx_alloc(size_t size) {
    if (size > FXCACHE_MAX) {
        if (size <= MEM_FXLARGE_MAX_SIZE) {
            return mem_fxlarge_alloc((uint8_t) ((size + PAGE_SIZE - 1) / PAGE_SIZE)); // just give them an entire page
        }
        ERRX_RAISE_GENERIC(GERR_REQUEST_TOO_LARGE);
        return false;
    }
    assert(!is_allocating);
    is_allocating = true;
    size = round_size(size);
    void *out = mem_fxcache_query(&fx_cache, size);
    if (out == NULL) {
        out = mem_fxseq_alloc(&fx_seq, size);
        if (out == NULL) {
            ERRX_TRACEPOINT;
        }
    }
    assert(is_allocating);
    is_allocating = false;
    return out;
}

void mem_fx_free(void *data, size_t size) {
    assert(size <= MEM_FXLARGE_MAX_SIZE);
    if (size > FXCACHE_MAX) {
        mem_fxlarge_free(data, (uint8_t) ((size + PAGE_SIZE - 1) / PAGE_SIZE));
    } else {
        size = round_size(size);
        mem_fxcache_insert(&fx_cache, data, size);
    }
}
