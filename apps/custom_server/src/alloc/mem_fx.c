#include "mem_fx.h"
#include "mem_fxcache.h"
#include "mem_fxseq.h"

static struct mem_fxcache fx_cache = MEM_FXCACHE_INIT;
static struct mem_fxseq fx_seq = MEM_FXSEQ_PREINIT;

seL4_Error mem_fx_init(void) {
    return mem_fxseq_init(&fx_seq);
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
    size = round_size(size);
    void *out = mem_fxcache_query(&fx_cache, size);
    if (out != NULL) {
        return out;
    } else {
        return mem_fxseq_alloc(&fx_seq, size);
    }
}

void mem_fx_free(void *data, size_t size) {
    size = round_size(size);
    mem_fxcache_insert(&fx_cache, data, size);
}
