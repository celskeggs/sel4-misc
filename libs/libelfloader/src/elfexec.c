#include <sel4/sel4.h>
#include <resource/cslot.h>
#include <elfloader/elfexec.h>
#include <elfloader/elfloader.h>
#include <elfloader/elfparser.h>
#include <elfloader/elfcontext.h>
#include <resource/mem_fx.h>

#define IPC_ADDRESS (0x40000 - PAGE_SIZE)

// privileged component

struct priv_cookie {
    object_token tcb;
};

// internal
static bool priv_registers_configure(struct priv_cookie *holder, uint32_t entry_vector, uintptr_t param) {
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

static bool priv_tcb_configure(struct priv_cookie *cookie, seL4_CPtr cspace, uint32_t ipc_address, seL4_CPtr fault_ep,
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

static struct priv_cookie *priv_init(seL4_CPtr cspace, uint32_t ipc_address, uint8_t priority, uint32_t entry_vector) {
    struct priv_cookie *cookie = mem_fx_alloc(sizeof(struct priv_cookie));
    if (cookie == NULL) {
        ERRX_TRACEPOINT;
        return NULL;
    }
    cookie->tcb = object_alloc(seL4_TCBObject);
    if (cookie->tcb == NULL) {
        ERRX_TRACEPOINT;
        mem_fx_free(cookie, sizeof(struct priv_cookie));
        return false;
    }
    if (!priv_tcb_configure(cookie, cspace, ipc_address, seL4_CapNull, priority)) {
        ERRX_TRACEPOINT;
        object_free_token(cookie->tcb);
        mem_fx_free(cookie, sizeof(struct priv_cookie));
        return false;
    }
    if (!priv_registers_configure(cookie, entry_vector, IPC_ADDRESS)) {
        ERRX_TRACEPOINT;
        object_free_token(cookie->tcb);
        mem_fx_free(cookie, sizeof(struct priv_cookie));
        return false;
    }
    return cookie;
}

static bool priv_start(struct priv_cookie *cookie) {
    assert(cookie != NULL);
    int err = seL4_TCB_Resume(object_cap(cookie->tcb));
    if (err != seL4_NoError) {
        ERRX_RAISE_SEL4(err);
        return false;
    }
    return true;
}

static void priv_stop(struct priv_cookie *cookie) {
    assert(cookie != NULL);
    assert(seL4_TCB_Suspend(object_cap(cookie->tcb)) == seL4_NoError);
}

static void priv_destroy(struct priv_cookie *cookie) {
    assert(cookie != NULL);
    object_free_token(cookie->tcb);
    cookie->tcb = NULL;
    mem_fx_free(cookie, sizeof(struct priv_cookie));
}

// unprivileged component

static bool cspace_configure(struct elfexec *holder, seL4_IA32_Page ipc_page, seL4_CPtr io_ep) {
    seL4_CNode cspace = object_cap(holder->cspace);
    if (!cslot_mutate(cspace, cspace, seL4_CapData_Guard_new(0, 32 - ECAP_ROOT_BITS))
        || !cslot_copy_out(object_cap(holder->page_directory), cspace, ecap_PD, 32)
        || !cslot_copy_out(cspace, cspace, ecap_CNode, 32)
        || !cslot_copy_out(ipc_page, cspace, ecap_IPC, 32)
        || !cslot_copy_out(io_ep, cspace, ecap_IOEP, 32)) {
        ERRX_TRACEPOINT;
        return false;
    }
    return true;
}

bool elfexec_init(void *elf, size_t file_size, struct elfexec *holder, seL4_CPtr fault_ep, uint8_t priority,
                  seL4_CPtr io_ep) {
    holder->page_directory = object_alloc(seL4_IA32_PageDirectoryObject);
    if (holder->page_directory == NULL) {
        ERRX_TRACEPOINT;
        return false;
    }
    holder->cspace = object_alloc(seL4_CapTableObject);
    if (holder->cspace == NULL) {
        object_free_token(holder->page_directory);
        ERRX_TRACEPOINT;
        return false;
    }

    holder->pd = elfloader_load(elf, file_size, object_cap(holder->page_directory));
    if (holder->pd == NULL) {
        elfexec_destroy(holder);
        ERRX_TRACEPOINT;
        return false;
    }
    seL4_IA32_Page ipc_page = elfloader_get_page(holder->pd, (void *) IPC_ADDRESS,
                                                 ELF_MEM_READABLE | ELF_MEM_WRITABLE, true);
    if (ipc_page == seL4_CapNull) {
        elfexec_destroy(holder);
        ERRX_TRACEPOINT;
        return false;
    }
    if (!cspace_configure(holder, ipc_page, io_ep)) {
        elfexec_destroy(holder);
        ERRX_TRACEPOINT;
        return false;
    }
    // other capability parameters such as page directory are pulled from the cspace
    holder->priv_cookie = priv_init(object_cap(holder->cspace), IPC_ADDRESS, priority, holder->pd->entry_position);
    if (holder->priv_cookie == NULL) {
        elfexec_destroy(holder);
        ERRX_TRACEPOINT;
        return false;
    }
    return true;
}

bool elfexec_start(struct elfexec *holder) {
    return priv_start(holder->priv_cookie);
}

void elfexec_stop(struct elfexec *holder) {
    priv_stop(holder->priv_cookie);
}

void elfexec_destroy(struct elfexec *holder) {
    if (holder->priv_cookie != NULL) {
        priv_stop(holder->priv_cookie);
        priv_destroy(holder->priv_cookie);
    }
    if (holder->pd != NULL) {
        elfloader_unload(holder->pd);
    }
    object_free_token(holder->cspace); // TODO: recursively free this first?
    object_free_token(holder->page_directory);
    holder->page_directory = holder->cspace = holder->priv_cookie = NULL;
    holder->pd = NULL;
}
