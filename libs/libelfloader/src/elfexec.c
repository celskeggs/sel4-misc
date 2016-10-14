#include <sel4/sel4.h>
#include <resource/cslot.h>
#include <elfloader/elfexec.h>
#include <elfloader/elfloader.h>
#include <elfloader/elfparser.h>
#include <elfloader/elfcontext.h>

#define IPC_ADDRESS (0x40000 - PAGE_SIZE)

static bool tcb_configure(struct elfexec *holder, seL4_CPtr fault_ep, uint8_t priority, seL4_IA32_Page ipc_page) {
    int err = seL4_TCB_Configure(object_cap(holder->tcb), fault_ep, priority, object_cap(holder->cspace),
                                 seL4_CapData_Guard_new(0, 32 - ECAP_ROOT_BITS),
                                 object_cap(holder->page_directory),
                                 (seL4_CapData_t) {.words = {0}}, IPC_ADDRESS, ipc_page);
    if (err != seL4_NoError) {
        ERRX_RAISE_SEL4(err);
        return false;
    }
    return true;
}

static bool registers_configure(struct elfexec *holder, uintptr_t param) {
    seL4_UserContext context;
    context.eip = holder->pd->entry_position;
    context.esp = 0; // populated by the new thread
    context.eflags = 0;
    context.eax = context.ecx = context.edx = context.esi = context.edi = context.ebp = 0;
    context.ebx = param;
    // note: we don't need to pass info on the loaded frames because __executable_start and _end are both available.
    context.tls_base = context.fs = context.gs = 0; // TODO: are these necessary?
    int err = seL4_TCB_WriteRegisters(object_cap(holder->tcb), false, 0, 13, &context);
    if (err != seL4_NoError) {
        ERRX_RAISE_SEL4(err);
        return false;
    }
    return true;
}

static bool cspace_configure(struct elfexec *holder, seL4_IA32_Page ipc_page, seL4_CPtr io_ep) {
    seL4_CNode cspace = object_cap(holder->cspace);
    if (!cslot_mutate(cspace, cspace, seL4_CapData_Guard_new(0, 32 - ECAP_ROOT_BITS))
        || !cslot_copy_out(object_cap(holder->page_directory), cspace, ecap_PD, 32)
        || !cslot_copy_out(cspace, cspace, ecap_CNode, 32)
        || !cslot_copy_out(ipc_page, cspace, ecap_IPC, 32)
        || !cslot_copy_out(io_ep, cspace, ecap_IOEP, 32)
            ) {
        ERRX_TRACEPOINT;
        return false;
    }
    return true;
}

bool elfexec_init(void *elf, size_t file_size, struct elfexec *holder, seL4_CPtr fault_ep, uint8_t priority,
                  seL4_CPtr io_ep) {
    int alloc_count = 3;
    int types[] = {seL4_IA32_PageDirectoryObject, seL4_TCBObject, seL4_CapTableObject};
    object_token *allocs[] = {&holder->page_directory, &holder->tcb, &holder->cspace};

    for (int i = 0; i < alloc_count; i++) {
        *allocs[i] = object_alloc(types[i]);
        if (allocs[i] == NULL) {
            while (--i >= 0) {
                object_free_token(allocs[i]);
            }
            ERRX_TRACEPOINT;
            return false;
        }
    }

    holder->pd = elfloader_load(elf, file_size, object_cap(holder->page_directory));
    if (holder->pd == NULL) {
        for (int i = 0; i < alloc_count; i++) {
            object_free_token(*allocs[i]);
        }
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
    if (!tcb_configure(holder, fault_ep, priority, ipc_page) || !registers_configure(holder, IPC_ADDRESS)
        || !cspace_configure(holder, ipc_page, io_ep)) {
        elfexec_destroy(holder);
        ERRX_TRACEPOINT;
        return false;
    }
    return true;
}

bool elfexec_start(struct elfexec *holder) {
    int err = seL4_TCB_Resume(object_cap(holder->tcb));
    if (err != seL4_NoError) {
        ERRX_RAISE_SEL4(err);
        return false;
    }
    return true;
}

void elfexec_stop(struct elfexec *holder) {
    assert(seL4_TCB_Suspend(object_cap(holder->tcb)) == seL4_NoError);
}

void elfexec_destroy(struct elfexec *holder) {
    elfexec_stop(holder);
    elfloader_unload(holder->pd);
    object_free_token(holder->tcb);
    object_free_token(holder->cspace); // TODO: recursively free this first?
    object_free_token(holder->page_directory);
    holder->page_directory = holder->cspace = holder->tcb = NULL;
    holder->pd = NULL;
}
