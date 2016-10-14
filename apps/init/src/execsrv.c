#include <bedrock/bedrock.h>
#include <elfloader/ctxexec.h>
#include <resource/object.h>
#include <resource/cslot.h>
#include <elfloader/elfcontext.h>
#include <resource/mem_fx.h>

struct priv_cookie {
    object_token tcb;
};

// internal
static bool exec_registers_configure(struct priv_cookie *holder, uint32_t entry_vector, uintptr_t param) {
    seL4_UserContext context;
    context.eip = entry_vector;
    context.esp = 0; // populated by the new thread
    context.eflags = 0;
    context.eax = context.ecx = context.edx = context.esi = context.edi = context.ebp = 0;
    context.ebx = param;
    // note: we don't need to pass info on the loaded frames because __executable_start and _end are both available.
    context.tls_base = context.fs = context.gs = 0;
    int err = seL4_TCB_WriteRegisters(object_cap(holder->tcb), false, 0, 13, &context);
    if (err != seL4_NoError) {
        ERRX_RAISE_SEL4(err);
        return false;
    }
    return true;
}

static bool exec_tcb_configure(struct priv_cookie *cookie, seL4_CPtr cspace, uint32_t ipc_address, seL4_CPtr fault_ep,
                               uint8_t priority) {
    seL4_CPtr slots = cslot_alloc_slab(2);
    if (slots == seL4_CapNull) {
        ERRX_TRACEPOINT;
        return false;
    }
    seL4_CPtr slot_pd = slots + 0, slot_ipc = slots + 1;
    if (!cslot_copy_in(cspace, ecap_PD, 32, slot_pd)
        || !cslot_copy_in(cspace, ecap_IPC, 32, slot_ipc)) {
        cslot_free(slot_pd);
        cslot_free(slot_ipc);
        ERRX_TRACEPOINT;
        return false;
    }
    int err = seL4_TCB_Configure(object_cap(cookie->tcb), fault_ep, priority, cspace, (seL4_CapData_t) {.words = {0}},
                                 slot_pd, (seL4_CapData_t) {.words = {0}}, ipc_address, slot_ipc);
    cslot_free(slot_pd);
    cslot_free(slot_ipc);
    if (err != seL4_NoError) {
        ERRX_RAISE_SEL4(err);
        return false;
    }
    return true;
}

uint32_t exec_init(seL4_CPtr cspace, uint32_t ipc_address, uint8_t priority, uint32_t entry_vector) {
    struct priv_cookie *cookie = mem_fx_alloc(sizeof(struct priv_cookie));
    if (cookie == NULL) {
        ERRX_TRACEPOINT;
        return 0;
    }
    cookie->tcb = object_alloc(seL4_TCBObject);
    if (cookie->tcb == NULL) {
        ERRX_TRACEPOINT;
        mem_fx_free(cookie, sizeof(struct priv_cookie));
        return 0;
    }
    if (!exec_tcb_configure(cookie, cspace, ipc_address, seL4_CapNull, priority)) {
        ERRX_TRACEPOINT;
        object_free_token(cookie->tcb);
        mem_fx_free(cookie, sizeof(struct priv_cookie));
        return 0;
    }
    if (!exec_registers_configure(cookie, entry_vector, ipc_address)) {
        ERRX_TRACEPOINT;
        object_free_token(cookie->tcb);
        mem_fx_free(cookie, sizeof(struct priv_cookie));
        return 0;
    }
    return (uint32_t) cookie;
}

bool exec_start(uint32_t cookie_r) {
    struct priv_cookie *cookie = (struct priv_cookie *) cookie_r;
    assert(cookie != NULL);
    int err = seL4_TCB_Resume(object_cap(cookie->tcb));
    if (err != seL4_NoError) {
        ERRX_RAISE_SEL4(err);
        return false;
    }
    return true;
}

void exec_stop(uint32_t cookie_r) {
    struct priv_cookie *cookie = (struct priv_cookie *) cookie_r;
    assert(cookie != NULL);
    assert(seL4_TCB_Suspend(object_cap(cookie->tcb)) == seL4_NoError);
}

void exec_destroy(uint32_t cookie_r) {
    struct priv_cookie *cookie = (struct priv_cookie *) cookie_r;
    assert(cookie != NULL);
    object_free_token(cookie->tcb);
    cookie->tcb = NULL;
    mem_fx_free(cookie, sizeof(struct priv_cookie));
}
