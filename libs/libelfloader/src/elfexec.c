
#include <sel4/types.h>
#include <resource/untyped.h>
#include <resource/cslot.h>
#include <elfloader/elfloader.h>
#include <elfloader/elfparser.h>

#define IPC_ADDRESS (0x40000 - PAGE_SIZE)

static untyped_4k_ref allocate_retyped(int type, int szb) {
    untyped_4k_ref ref = untyped_allocate_4k();
    if (ref == NULL) {
        ERRX_TRACEPOINT;
        return NULL;
    }
    if (!cslot_retype(untyped_ptr_4k(ref), type, 0, szb, untyped_auxptr_4k(ref), 1)) {
        ERRX_TRACEPOINT;
        return NULL;
    }
    return ref;
}

static bool allocate_retypeds(int count, int *types, int *szb, untyped_4k_ref **outs) {
    for (int i = 0; i < count; i++) {
        *outs[i] = allocate_retyped(types[i], szb[i]);
        if (outs[i] == NULL) {
            while (--i >= 0) {
                untyped_free_4k(outs[i]);
            }
            ERRX_TRACEPOINT;
            return false;
        }
    }
    return true;
}

// TODO: get 4 (log 16, the cnode entry size) as a constant
#define CAPBITS BITS_4KIB - 4

bool elfexec_init(void *elf, size_t file_size, struct elfexec *holder, seL4_CPtr fault_ep, uint8_t priority) {
    int alloc_count = 3;
    int types[] = {seL4_IA32_PageDirectoryObject, seL4_TCBObject, seL4_CapTableObject};
    int bits[] = {0, 0, CAPBITS};
    untyped_4k_ref *allocs[] = {&holder->page_directory, &holder->tcb, &holder->cspace};
    if (!allocate_retypeds(alloc_count, types, bits, allocs)) {
        ERRX_TRACEPOINT;
        return false;
    }
    holder->pd = elfloader_load(elf, file_size, untyped_auxptr_4k(holder->page_directory));
    if (holder->pd == NULL) {
        for (int i = 0; i < alloc_count; i++) {
            untyped_free_4k(*allocs[i]);
        }
        ERRX_TRACEPOINT;
        return false;
    }
    seL4_IA32_Page ipc_page = elfloader_get_page(holder->pd, (void *) IPC_ADDRESS, ELF_MEM_READABLE | ELF_MEM_EXECUTABLE, true);
    if (ipc_page == seL4_CapNull) {
        elfloader_unload(holder->pd);
        for (int i = 0; i < alloc_count; i++) {
            untyped_free_4k(*allocs[i]);
        }
        ERRX_TRACEPOINT;
        return false;
    }
    seL4_TCB tcb = untyped_auxptr_4k(holder->tcb);
    int err = seL4_TCB_Configure(tcb, fault_ep, priority, untyped_auxptr_4k(holder->cspace),
                                 seL4_CapData_Guard_new(0, 32 - CAPBITS), untyped_auxptr_4k(holder->page_directory), {},
                                 IPC_ADDRESS, ipc_page);
    if (err != seL4_NoError) {
        elfloader_unload(holder->pd);
        for (int i = 0; i < alloc_count; i++) {
            untyped_free_4k(*allocs[i]);
        }
        ERRX_RAISE_SEL4(err);
        return false;
    }
    seL4_UserContext context;
    context.eip = holder->pd->entry_position;
    context.esp = 0; // populated by the new thread
    context.eflags = 0;
    context.eax = context.ecx = context.edx = context.esi = context.edi = context.ebp = 0;
    context.ebx = param; // TODO: what should this be?
    // note: we don't need to pass info on the loaded frames because __executable_start and _end are both available.
    // we just need to pass the cspace setup - and maybe not even that?
    // TODO: set up cspace
    context.tls_base = context.fs = context.gs = 0; // TODO: are these necessary?
    err = seL4_TCB_WriteRegisters(tcb, false, 0, 13, &context);
    if (err != seL4_NoError) {
        elfloader_unload(holder->pd);
        for (int i = 0; i < alloc_count; i++) {
            untyped_free_4k(*allocs[i]);
        }
        ERRX_RAISE_SEL4(err);
        return false;
    }
    return true;
}

bool elfexec_start(struct elfexec *holder) {
    int err = seL4_TCB_Resume(untyped_auxptr_4k(holder->tcb));
    if (err != seL4_NoError) {
        ERRX_RAISE_SEL4(err);
        return false;
    }
    return true;
}

void elfexec_stop(struct elfexec *holder) {
    assert(seL4_TCB_Suspend(untyped_auxptr_4k(holder->tcb)) == seL4_NoError);
}

void elfexec_destroy(struct elfexec *holder) {
    elfexec_stop(holder);
    elfloader_unload(holder->pd);
    untyped_free_4k(holder->tcb);
    untyped_free_4k(holder->cspace); // TODO: recursively free this first?
    untyped_free_4k(holder->page_directory);
}