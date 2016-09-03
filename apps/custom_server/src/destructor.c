#include "destructor.h"
#include "alloc/mem_ao.h"

// destructors both allocated via mem_ao (cached here) or stack-allocated
static struct destructor_ctx *free_list = NULL;

static struct destructor_ctx *alloc(void) {
    struct destructor_ctx *ctx;
    if (free_list == NULL) {
        ctx = mem_ao_alloc(sizeof(struct destructor_ctx));
        if (ctx == NULL) {
            return NULL;
        }
    } else {
        ctx = free_list;
        free_list = ctx->next_ctx;
    }
    ctx->next_ctx = NULL;
    ctx->next_id = 0;
    return ctx;
}

static void dealloc(struct destructor_ctx *ctx) {
    ctx->next_ctx = free_list;
    free_list = ctx;
}

void destructor_destruct(struct destructor_ctx *ctx) {
    assert(ctx != NULL);
    if (ctx->next_ctx != NULL) {
        destructor_destruct(ctx->next_ctx);
        dealloc(ctx->next_ctx);
        ctx->next_ctx = NULL;
    }
    while (ctx->next_id) {
        int free_id = --ctx->next_id;
        ctx->destructor_cbs[free_id](ctx->destructor_data[free_id]);
    }
    // note: this is reset to a working state at the end
}

bool destructor_insert(struct destructor_ctx *ctx, destructor_cb cb, void *data) {
    // TODO: allow NULL to represent "don't bother being able to destroy this"?
    assert(ctx != NULL);
    if (ctx->next_id >= DESTRUCTOR_COUNT) {
        struct destructor_ctx *subctx = alloc();
        if (subctx == NULL) {
            return false;
        }
        ctx->next_ctx = subctx;
        ctx = subctx;
        assert(ctx->next_id == 0);
    }
    ctx->destructor_cbs[ctx->next_id] = cb;
    ctx->destructor_data[ctx->next_id] = data;
    ctx->next_id++;
    return true;
}
