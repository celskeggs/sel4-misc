#include "mem_ao.h"

// 256 KB of static memory. TODO: find better ways than this
static uint8_t memory[256 * 1024];
static uint8_t *here = memory;

void *mem_ao_alloc(size_t len) {
    assert(here >= memory);
    if (here - memory + len <= sizeof(memory)) {
        void *out = here;
        here += len;
        return out;
    }
    return NULL;
}

void mem_ao_dealloc_last(void *ptr, size_t len) {
    uint8_t *hptr = ptr;
    assert(hptr + len == here);
    assert(hptr >= memory);
    assert(hptr - memory + len <= sizeof(memory));
    here = hptr;
}
