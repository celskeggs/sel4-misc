#include <sel4/sel4.h>
#include <elfloader/elfexec.h>
#include <resource/cslot.h>
#include <ipc/ipc.h>
#include <elfloader/ctxexec.h>

extern char *image_sandbox;
extern char *image_sandbox_end;

bool ipc_handle_ping(uint32_t sender, struct ipc_in_ping *in, struct ipc_out_ping *out) {
    (void) sender;
    out->value_neg = ~in->value;
    return true;
}

bool ipc_handle_alloc(uint32_t sender, seL4_CPtr cap_out, struct ipc_in_alloc *in,
                      struct ipc_out_alloc *out) {
    (void) sender;
    switch (in->object_type) {
        case seL4_CapTableObject:
        case seL4_IA32_4K:
        case seL4_IA32_PageTableObject:
        case seL4_IA32_PageDirectoryObject:
        case seL4_EndpointObject:
        case seL4_NotificationObject: {
            object_token ref = object_alloc(in->object_type);
            if (ref == NULL) {
                ERRX_TRACEPOINT;
                return false;
            }
            out->cookie = (uint32_t) ref; // TODO: something less insecure
            if (!cslot_copy(object_cap(ref), cap_out)) {
                ERRX_TRACEPOINT;
                return false;
            }
            return true;
        }
        default: {
            ERRX_RAISE_GENERIC(GERR_UNSUPPORTED_OPTION);
            return false;
        }
    }
}

bool ipc_handle_free(uint32_t sender, struct ipc_in_free *in, struct ipc_out_free *out) {
    (void) sender;
    (void) out;
    assert(in->cookie != 0);
    object_token tok = (object_token) in->cookie;
    object_free_token(tok);
    return true;
}

bool ipc_handle_proc_init(uint32_t sender, seL4_CPtr cspace, struct ipc_in_proc_init *in,
                          struct ipc_out_proc_init *out) {
    (void) sender;
    // TODO: use more secure cookies
    out->cookie = exec_init(cspace, in->ipc_addr, in->pri, in->entry_vector);
    if (out->cookie == 0) {
        ERRX_TRACEPOINT;
        return false;
    }
    return true;
}

bool ipc_handle_proc_start(uint32_t sender, struct ipc_in_proc_start *in, struct ipc_out_proc_start *out) {
    (void) sender;
    (void) out;
    if (!exec_start(in->cookie)) {
        ERRX_TRACEPOINT;
        return false;
    }
    return true;
}

bool ipc_handle_proc_stop(uint32_t sender, struct ipc_in_proc_stop *in, struct ipc_out_proc_stop *out) {
    (void) sender;
    (void) out;
    exec_stop(in->cookie);
    return true;
}

bool ipc_handle_proc_destroy(uint32_t sender, struct ipc_in_proc_destroy *in, struct ipc_out_proc_destroy *out) {
    (void) sender;
    (void) out;
    exec_destroy(in->cookie);
    return true;
}

bool main(void) {
    object_token token = object_alloc(seL4_EndpointObject);
    if (token == NULL) {
        ERRX_TRACEPOINT;
        return false;
    }
    seL4_CPtr root_endpoint = object_cap(token);
    struct elfexec context;
    if (!elfexec_init(image_sandbox, image_sandbox_end - image_sandbox, &context, 255, root_endpoint)) {
        ERRX_TRACEPOINT;
        return false;
    }
    if (!elfexec_start(&context)) {
        ERRX_TRACEPOINT;
        return false;
    }
    SERVER_LOOP(root_endpoint, true, SERVER_FOR(ping)
            SERVER_FOR(alloc)
            SERVER_FOR(free)
            SERVER_FOR(proc_init)
            SERVER_FOR(proc_start)
            SERVER_FOR(proc_stop)
            SERVER_FOR(proc_destroy));
}
