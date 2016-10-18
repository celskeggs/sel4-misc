#ifndef APP_REGISTRAR_HASHMAP_H
#define APP_REGISTRAR_HASHMAP_H

#include <bedrock/bedrock.h>
#include <resource/mem_vspace.h>

// uses a fixed 4096-byte bucket set, meaning 1024 buckets exactly.

#define HASHMAP_BUCKETS (PAGE_SIZE / sizeof(struct hashbucket *))

struct hashbucket {
    const char *key;
    void *value;
    struct hashbucket *next;
};

struct hashmap {
    size_t entry_count;
    struct hashbucket **buckets;
};

struct hashmap *hashmap_new(void);

size_t hashmap_length(struct hashmap *map);

void *hashmap_get(struct hashmap *map, const char *key);

// uses errx on result. puts previous value in *last, if last != NULL.
bool hashmap_set(struct hashmap *map, const char *key, void *value, void **last);

void *hashmap_remove(struct hashmap *map, const char *key);

void hashmap_free(struct hashmap *map);

const char *hashmap_iter_first(struct hashmap *map);

const char *hashmap_iter_next(struct hashmap *map, const char *last);

#endif //APP_REGISTRAR_HASHMAP_H
