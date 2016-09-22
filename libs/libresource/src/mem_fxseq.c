#include <resource/mem_fxseq.h>

bool mem_fxseq_init(struct mem_fxseq *fxseq) {
    if (!mem_fxalloc_create(&fxseq->prealloc, MEM_FXSEQ_INITIAL_CHUNK * 2)) {
        ERRX_TRACEPOINT;
        return false;
    }
    fxseq->ready = &fxseq->prealloc;
    fxseq->active = NULL;
    return true;
}

// we're allocating each new pool before necessary just so that we don't run out. maybe this can be more efficient.
static bool expand_pool(struct mem_fxseq *fxseq) {
    if (fxseq->ready == NULL) {
        ERRX_RAISE_GENERIC(GERR_INVALID_STATE); // not initialized yet, or currently being reinitialized here.
        return false;
    }
    fxseq->active = fxseq->ready; // should make allocations possible again
    fxseq->ready = NULL; // make sure this section doesn't recurse
    struct mem_fxalloc *next = (struct mem_fxalloc *) mem_fxalloc_alloc(fxseq->active, sizeof(struct mem_fxalloc));
    if (next == NULL) {
        ERRX_TRACEPOINT;
        return false; // could not set up memory stuff
    }
    if (!mem_fxalloc_create(next, mem_fxalloc_size(fxseq->active) * 2)) {
        // TODO: don't leak 'next'
        ERRX_TRACEPOINT;
        return false;
    }
    fxseq->ready = next;
    return true;
}

void *mem_fxseq_alloc(struct mem_fxseq *fxseq, size_t size) {
    assert(size > 0 && size <= MEM_FXSEQ_MAX_ALLOC);
    if (fxseq->active == NULL) {
        if (fxseq->current == NULL) {
            fxseq->current = fxseq->initial_data;
        }
        if (fxseq->current + size <= (void*) fxseq->initial_data + MEM_FXSEQ_INITIAL_CHUNK) {
            // we can just take from our fixed pool
            void *out = fxseq->current;
            fxseq->current += size;
            return out;
        }
        // fixed pool is over; time to switch to dynamic pool
        if (!expand_pool(fxseq)) {
            ERRX_TRACEPOINT;
            return NULL;
        }
    }
    // take from an allocator
    void *out = mem_fxalloc_alloc(fxseq->active, size);
    if (out != NULL) {
        return out;
    } else if (!mem_fxalloc_is_full(fxseq->active)) {
        // not full, so it failed for some other reason - expanding the pool won't help
        ERRX_TRACEPOINT;
        return NULL;
    } else {
        ERRX_CONSUME;
        // full, so we need to expand the pool
        if (!expand_pool(fxseq)) {
            ERRX_TRACEPOINT;
            return NULL;
        }
        // if this doesn't work, then there's no point trying anything else.
        out = mem_fxalloc_alloc(fxseq->active, size);
        if (out == NULL) {
            assert(!mem_fxalloc_is_full(out));
        }
        return out;
    }
}
