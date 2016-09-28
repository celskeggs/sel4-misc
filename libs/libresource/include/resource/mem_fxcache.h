#ifndef LIB_RESOURCE_MEM_FXCACHE_H
#define LIB_RESOURCE_MEM_FXCACHE_H

#include <bedrock/bedrock.h>

// this is a "frontend" for an allocation-only allocator that allows reuse for a limited set of sizes

#define FXCACHE_UNIT (sizeof(struct mem_fxcache_free))
#define FXCACHE_RANGE 512
#define FXCACHE_MAX (FXCACHE_RANGE * FXCACHE_UNIT)

struct mem_fxcache_free {
    struct mem_fxcache_free *next;
};

struct mem_fxcache {
    struct mem_fxcache_free *heads[FXCACHE_RANGE];
};

#define MEM_FXCACHE_INIT ((struct mem_fxcache) { .heads = {NULL /* should autofill */} })

static inline void mem_fxcache_init(struct mem_fxcache *cache) {
    *cache = MEM_FXCACHE_INIT;
}

void mem_fxcache_insert(struct mem_fxcache *cache, void *ptr, size_t size);

void *mem_fxcache_query(struct mem_fxcache *cache, size_t size);

#endif //LIB_RESOURCE_MEM_FXCACHE_H
