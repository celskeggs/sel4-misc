#include <bedrock/debug.h>
#include <init/init.h>
#include <ipc/ipc.h>
#include <resource/mem_page.h>
#include <elfloader/elfexec.h>

extern char *image_registrar;
extern char *image_registrar_end;

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

bool main(void) {
    debug_println("hello, sandbox world!");
    object_token ep = object_alloc(seL4_EndpointObject);
    if (ep == NULL) {
        ERRX_TRACEPOINT;
        return false;
    }
    seL4_CPtr minted = cslot_alloc();
    if (minted == seL4_CapNull) {
        object_free_token(ep);
        ERRX_TRACEPOINT;
        return false;
    }
    if (!cslot_mint(object_cap(ep), minted, 1123)) {
        cslot_free(minted);
        object_free_token(ep);
        ERRX_TRACEPOINT;
        return false;
    }
    struct elfexec context;
    if (!elfexec_init(image_registrar, image_registrar_end - image_registrar, &context, 255, minted)) {
        cslot_free(minted);
        object_free_token(ep);
        ERRX_TRACEPOINT;
        return false;
    }
    cslot_free(minted);
    if (!elfexec_start(&context)) {
        object_free_token(ep);
        ERRX_TRACEPOINT;
        return false;
    }
    DEBUG("start");
    seL4_CPtr cap = object_cap(ep);
    SERVER_LOOP(cap, true, SERVER_FOR(ping)
            SERVER_FOR(alloc)
            SERVER_FOR(free)
            SERVER_FOR(sandbox_ready))
}
