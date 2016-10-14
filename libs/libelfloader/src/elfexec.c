#include <sel4/sel4.h>
#include <resource/cslot.h>
#include <elfloader/elfexec.h>
#include <elfloader/elfloader.h>
#include <elfloader/elfparser.h>
#include <elfloader/elfcontext.h>
#include <elfloader/ctxexec.h>

#define IPC_ADDRESS (0x40000 - PAGE_SIZE)

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

bool elfexec_init(void *elf, size_t file_size, struct elfexec *holder, uint8_t priority, seL4_CPtr io_ep) {
    if (io_ep == seL4_CapNull || elf == NULL || holder == NULL) {
        ERRX_RAISE_GENERIC(GERR_NULL_VALUE);
        return false;
    }
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
    holder->priv_cookie = 0;
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
    holder->priv_cookie = exec_init(object_cap(holder->cspace), IPC_ADDRESS, priority, holder->pd->entry_position);
    if (!holder->priv_cookie) {
        elfexec_destroy(holder);
        ERRX_TRACEPOINT;
        return false;
    }
    return true;
}

bool elfexec_start(struct elfexec *holder) {
    return exec_start(holder->priv_cookie);
}

void elfexec_stop(struct elfexec *holder) {
    exec_stop(holder->priv_cookie);
}

void elfexec_destroy(struct elfexec *holder) {
    if (holder->priv_cookie) {
        exec_stop(holder->priv_cookie);
        exec_destroy(holder->priv_cookie);
    }
    if (holder->pd != NULL) {
        elfloader_unload(holder->pd);
    }
    object_free_token(holder->cspace); // TODO: recursively free this first?
    object_free_token(holder->page_directory);
    holder->page_directory = holder->cspace = NULL;
    holder->priv_cookie = 0;
    holder->pd = NULL;
}
