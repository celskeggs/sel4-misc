#include <resource/mem_fxcache.h>

static inline uint32_t get_linked_list_index_for_size(size_t size) {
    assert(FXCACHE_UNIT <= size && size <= FXCACHE_MAX);
    assert((size & (FXCACHE_UNIT - 1)) == 0);
    uint32_t ll_index = size / FXCACHE_UNIT;
    assert(ll_index < FXCACHE_MAX);
    return ll_index;
}

void mem_fxcache_insert(struct mem_fxcache *cache, void *ptr, size_t size) {
    uint32_t index = get_linked_list_index_for_size(size);
    struct mem_fxcache_free *node = (struct mem_fxcache_free *) ptr;
    node->next = cache->heads[index];
    cache->heads[index] = node;
}

void *mem_fxcache_query(struct mem_fxcache *cache, size_t size) {
    uint32_t index = get_linked_list_index_for_size(size);
    struct mem_fxcache_free *node = cache->heads[index];
    if (node != NULL) {
        cache->heads[index] = node->next;
        node->next = NULL;
    }
    return node;
}
