#include <resource/object.h>
#include <ipc/ipc.h>
#include <elfloader/elfcontext.h>
#include <resource/cslot.h>
#include <resource/mem_fx.h>

struct token {
    uint32_t cookie;
    seL4_CPtr slot;
};

object_token object_alloc(uint32_t object_type) {
    struct token *token = mem_fx_alloc(sizeof(struct token));
    if (token == NULL) {
        ERRX_TRACEPOINT;
        return NULL;
    }
    token->slot = cslot_alloc();
    if (token->slot == seL4_CapNull) {
        mem_fx_free(token, sizeof(struct token));
        ERRX_TRACEPOINT;
        return NULL;
    }
    struct ipc_out_alloc out;
    if (!ipc_alloc(ecap_IOEP, token->slot, &(struct ipc_in_alloc) {.object_type=object_type}, &out)) {
        cslot_free(token->slot);
        mem_fx_free(token, sizeof(struct token));
        return NULL;
    }
    return token;
}

seL4_CPtr object_cap(object_token token_r) {
    struct token *token = (struct token *) token_r;
    assert(token != NULL && token->slot != seL4_CapNull);
    return token->slot;
}

void object_free_token(object_token token_r) {
    struct token *token = (struct token *) token_r;
    assert(token != NULL && token->slot != seL4_CapNull);
    cslot_free(token->slot);
    token->slot = seL4_CapNull;
    struct ipc_out_free out;
    // TODO: another solution besides asserting... maybe cache these...?
    assert(ipc_free(ecap_IOEP, &(struct ipc_in_free) {.cookie = token->cookie}, &out));
    mem_fx_free(token, sizeof(struct token));
}
