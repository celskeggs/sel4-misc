#include "mem.h"

#define TINY_BITS 4
#define TINY_SIZE (1 << TINY_BITS)
#define MIN_BITS (TINY_BITS + 1)
#define MIN_SIZE (1 << MIN_BITS)
#define MAX_BITS 12
#define MAX_SIZE (1 << MAX_BITS)
#define BIT_BUCKETS (MAX_BITS - TINY_BITS)

struct free_bucket {
    struct free_bucket *left;
    struct free_bucket *right;
};

seL4_CompileTimeAssert(sizeof(struct free_bucket) <= MIN_SIZE);

void *mem_alloc(size_t len) {
    assert(len >= 1);
    if (len <= MAX_SIZE) {
        int bits;
        if (len <= TINY_SIZE) {
            // round up to 16 bytes (1 << TINY_BITS) and allocate from slab.
            slab_alloc()
        } else {
            bits = 32 - __builtin_clz((uint8_t) len - 1); // round up to 1 << bits = rounded up to nearest power of 2
        }

    }
}

void mem_dealloc(void *ptr, size_t len) {

}