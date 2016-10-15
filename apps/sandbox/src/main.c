#include <bedrock/debug.h>
#include <init/init.h>
#include <ipc/ipc.h>
#include <resource/mem_page.h>
#include <elfloader/elfexec.h>
#include <resource/mem_fx.h>

extern char *image_registrar;
extern char *image_registrar_end;
seL4_CPtr sandbox_ep = seL4_CapNull;

bool ipc_handle_ping(uint32_t sender, struct ipc_in_ping *in, struct ipc_out_ping *out) {
    (void) sender;
    out->value_neg = ~in->value;
    return true;
}

bool ipc_handle_alloc(uint32_t sender, seL4_CPtr cap_out, struct ipc_in_alloc *in, struct ipc_out_alloc *out) {
    if (!ipc_alloc(ecap_IOEP, cap_out, in, out)) {
        ERRX_TRACEPOINT;
        return false;
    }
    return true;
}

bool ipc_handle_free(uint32_t sender, struct ipc_in_free *in, struct ipc_out_free *out) {
    if (!ipc_free(ecap_IOEP, in, out)) {
        ERRX_TRACEPOINT;
        return false;
    }
    return true;
}

bool ipc_handle_sandbox_ready(uint32_t sender, seL4_CPtr cap_in, struct ipc_in_sandbox_ready *in, struct ipc_out_sandbox_ready *out) {
    debug_printdec(sender);
    ERRX_RAISE_GENERIC(GERR_UNKNOWN_ERROR);
    return false;
}

struct elfexec *launch_process(void *image, size_t length, uint8_t priority, uint32_t badge_number) {
    struct elfexec *context = mem_fx_alloc(sizeof(struct elfexec));
    if (context == NULL) {
        ERRX_TRACEPOINT;
        return NULL;
    }
    seL4_CPtr minted = cslot_alloc();
    if (minted == seL4_CapNull) {
        ERRX_TRACEPOINT;
        mem_fx_free(context, sizeof(struct elfexec));
        return NULL;
    }
    if (!cslot_mint(sandbox_ep, minted, badge_number)) {
        cslot_free(minted);
        mem_fx_free(context, sizeof(struct elfexec));
        ERRX_TRACEPOINT;
        return NULL;
    }
    if (!elfexec_init(image_registrar, image_registrar_end - image_registrar, context, priority, minted)) {
        cslot_free(minted);
        mem_fx_free(context, sizeof(struct elfexec));
        ERRX_TRACEPOINT;
        return NULL;
    }
    cslot_free(minted);
    if (!elfexec_start(context)) {
        elfexec_destroy(context);
        mem_fx_free(context, sizeof(struct elfexec));
        ERRX_TRACEPOINT;
        return NULL;
    }
    return context;
}

bool main(void) {
    debug_println("hello, sandbox world!");
    object_token ep = object_alloc(seL4_EndpointObject);
    if (ep == NULL) {
        ERRX_TRACEPOINT;
        return false;
    }
    sandbox_ep = object_cap(ep);
    // throwing away registrar pointer - not going to free it.
    if (launch_process(image_registrar, image_registrar_end - image_registrar, 255, 1) == NULL) {
        sandbox_ep = seL4_CapNull;
        object_free_token(ep);
        ERRX_TRACEPOINT;
        return false;
    }
    SERVER_LOOP(sandbox_ep, true, SERVER_FOR(ping)
            SERVER_FOR(alloc)
            SERVER_FOR(free)
            SERVER_FOR(sandbox_ready))
}
