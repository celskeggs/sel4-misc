#ifndef SEL4_MISC_MEM_FXSEQ_H
#define SEL4_MISC_MEM_FXSEQ_H

#include <bedrock/errx.h> // this module uses errx
#include "mem_page.h"
#include "mem_fxalloc.h"

#define MEM_FXSEQ_MAX_ALLOC PAGE_SIZE
#define MEM_FXSEQ_INITIAL_CHUNK (PAGE_SIZE * 16)

struct mem_fxseq {
    void *current;
    uint8_t initial_data[MEM_FXSEQ_INITIAL_CHUNK];
    struct mem_fxalloc *active;
    struct mem_fxalloc *ready;
    struct mem_fxalloc prealloc;
};

// use this as an initializer
#define MEM_FXSEQ_PREINIT ((struct mem_fxseq) {.current = NULL, .ready = NULL, .active = NULL})

// does not currently have a way to be deinitialized, but maybe TODO it should?
// NOTE: this system WILL WORK before being initialized! this is purposeful, for bootstrapping reasons. allocation will
// be limited, however, until that point.
bool mem_fxseq_init(struct mem_fxseq *fxseq);

void *mem_fxseq_alloc(struct mem_fxseq *fxseq, size_t size);

#endif //SEL4_MISC_MEM_FXSEQ_H
