#include <resource/mem_fx.h>
#include <resource/mem_page.h>
#include "hashmap.h"

struct hashmap *hashmap_new() {
    struct hashmap *hm = mem_fx_alloc(sizeof(struct hashbucket));
    if (hm == NULL) {
        ERRX_TRACEPOINT;
        return false;
    }
    hm->buckets = mem_fx_alloc(PAGE_SIZE);
    if (hm->buckets == NULL) {
        mem_fx_free(hm, sizeof(struct hashbucket));
        ERRX_TRACEPOINT;
        return false;
    }
    hm->entry_count = 0;
    memset(hm->buckets, 0, PAGE_SIZE);
    return hm;
}

size_t hashmap_length(struct hashmap *map) {
    return map->entry_count;
}

static uint32_t get_hash_for(const char *key) {
    // uses a FNV-1a hash
    uint32_t hash = 2166136261;
    while (*key) {
        hash = hash ^ *key;
        hash = hash * 16777619;
        key++;
    }
    return ((hash / HASHMAP_BUCKETS) ^ (hash)) & (HASHMAP_BUCKETS - 1);
}

static struct hashbucket *get_bucket_for(struct hashmap *map, const char *key) {
    uint32_t wrapped_hash = get_hash_for(key);
    assert(wrapped_hash < HASHMAP_BUCKETS);
    return map->buckets[wrapped_hash];
}

static void set_bucket_for(struct hashmap *map, const char *key, struct hashbucket *value) {
    uint32_t wrapped_hash = get_hash_for(key);
    assert(wrapped_hash < HASHMAP_BUCKETS);
    map->buckets[wrapped_hash] = value;
}

void *hashmap_get(struct hashmap *map, const char *key) {
    assert(key != NULL);
    struct hashbucket *bucket = get_bucket_for(map, key);
    while (bucket != NULL) {
        if (streq(bucket->key, key)) {
            return bucket->value;
        }
        bucket = bucket->next;
    }
    return NULL;
}

bool hashmap_set(struct hashmap *map, const char *key, void *value, void **last) {
    assert(key != NULL && value != NULL);
    struct hashbucket *bucket_o = get_bucket_for(map, key);
    struct hashbucket *bucket = bucket_o;
    while (bucket != NULL) {
        if (streq(bucket->key, key)) {
            if (last != NULL) {
                *last = bucket->value;
            }
            bucket->key = key; // ensure that we don't use keys after free
            bucket->value = value;
            return true;
        }
        bucket = bucket->next;
    }
    if (last != NULL) {
        *last = NULL;
    }
    bucket = mem_fx_alloc(sizeof(struct hashbucket));
    if (bucket == NULL) {
        ERRX_TRACEPOINT;
        return false;
    }
    bucket->next = bucket_o;
    bucket->key = key;
    bucket->value = value;
    set_bucket_for(map, key, bucket);
    return true;
}

void *hashmap_remove(struct hashmap *map, const char *key) {
    assert(key != NULL);
    struct hashbucket *bucket = get_bucket_for(map, key);
    struct hashbucket *last = NULL;
    while (bucket != NULL) {
        if (streq(bucket->key, key)) {
            if (last == NULL) {
                set_bucket_for(map, key, bucket->next);
            } else {
                last->next = bucket->next;
            }
            void *value = bucket->value;
            mem_fx_free(bucket, sizeof(struct hashbucket));
            return value;
        }
        last = bucket;
        bucket = bucket->next;
    }
    return NULL;
}

void hashmap_free(struct hashmap *map) {
    for (uint32_t i = 0; i < HASHMAP_BUCKETS; i++) {
        struct hashbucket *bucket = map->buckets[i];
        while (bucket != NULL) {
            void *to_free = bucket;
            bucket = bucket->next;
            mem_fx_free(to_free, sizeof(struct hashbucket));
        }
        map->buckets[i] = NULL;
    }
    mem_fx_free(map->buckets, PAGE_SIZE);
    mem_fx_free(map, sizeof(struct hashmap));
}

const char *hashmap_iter_first(struct hashmap *map) {
    for (uint32_t i = 0; i < HASHMAP_BUCKETS; i++) {
        if (map->buckets[i] != NULL) {
            return map->buckets[i]->key;
        }
    }
    return NULL;
}

const char *hashmap_iter_next(struct hashmap *map, const char *last) {
    assert(last != NULL);
    uint32_t wrapped_hash = get_hash_for(last);
    assert(wrapped_hash < HASHMAP_BUCKETS);
    struct hashbucket *bucket = map->buckets[wrapped_hash];
    assert(bucket != NULL); // otherwise, iteration is being done concurrently with modification or is otherwise borked
    if (bucket->next != NULL) {
        return bucket->next->key;
    } else {
        for (uint32_t i = wrapped_hash + 1; i < HASHMAP_BUCKETS; i++) {
            if (map->buckets[i] != NULL) {
                return map->buckets[i]->key;
            }
        }
        return NULL;
    }
}
