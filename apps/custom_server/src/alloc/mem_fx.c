#include "mem_fx.h"
#include "mem_ao.h"
#include "mem_fxalloc.h"
#include "mem_fxcache.h"

static bool fx_initialized = false;
static bool fx_initializing = false;
static struct mem_fxcache fx_cache = MEM_FXCACHE_INIT;
static struct mem_fxalloc fx_alloc;

seL4_Error mem_fx_init(void) {
    if (fx_initialized) {
        return seL4_NoError;
    }
    if (fx_initializing) {
        return seL4_IllegalOperation;
    }
    fx_initializing = true;
    seL4_Error err = mem_fxalloc_create(&fx_alloc);
    fx_initializing = false;
    assert(!fx_initialized);
    if (err != seL4_NoError) {
        return err;
    }
    fx_initialized = true;
    return seL4_NoError;
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
    } else if (fx_initialized) {
        return mem_fxalloc_alloc(&fx_alloc, size);
    } else {
        return mem_ao_alloc(size);
    }
}

void mem_fx_free(void *data, size_t size) {
    size = round_size(size);
    mem_fxcache_insert(&fx_cache, data, size);
}
