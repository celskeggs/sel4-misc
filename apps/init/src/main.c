#include <sel4/sel4.h>
#include <elfloader/elfexec.h>
#include <resource/cslot.h>
#include <resource/mem_vspace.h>
#include <resource/mem_fx.h>
#include <ipc/ipc.h>
#include <resource/object.h>

extern char *image_sandbox;
extern char *image_sandbox_end;

bool ipc_handle_ping(uint32_t sender, struct ipc_in_ping *in, struct ipc_out_ping *out) {
    (void) sender;
    out->value_neg = ~in->value;
    return true;
}

bool ipc_handle_init_alloc(uint32_t sender, seL4_CPtr cap_out, struct ipc_in_init_alloc *in,
                           struct ipc_out_init_alloc *out) {
    (void) sender;
    switch (in->object_type) {
        case seL4_CapTableObject: {
            untyped_4k_ref ref = untyped_allocate_retyped(in->object_type, CNODE_4K_BITS);
            out->cookie = (uint32_t) ref; // TODO: something less insecure
            return cslot_copy(untyped_auxptr_4k(ref), cap_out);
        }
        case seL4_IA32_4K:
        case seL4_IA32_PageTableObject: {
            untyped_4k_ref ref = untyped_allocate_retyped(in->object_type, 0);
            out->cookie = (uint32_t) ref; // TODO: something less insecure
            return cslot_copy(untyped_auxptr_4k(ref), cap_out);
        }
        default: {
            ERRX_RAISE_GENERIC(GERR_UNSUPPORTED_OPTION);
            return false;
        }
    }
}

bool main(void) {
    seL4_CPtr root_endpoint = object_alloc_endpoint();
    if (root_endpoint == seL4_CapNull) {
        return false;
    }
    struct elfexec context;
    if (!elfexec_init(image_sandbox, image_sandbox_end - image_sandbox, &context, seL4_CapNull, 255, root_endpoint)) {
        return false;
    }
    if (!elfexec_start(&context)) {
        return false;
    }
    SERVER_LOOP(root_endpoint, true, SERVER_FOR(ping)
            SERVER_FOR(init_alloc));
}

extern char __executable_start;
// referenced from mem_page.c
seL4_CPtr current_vspace = seL4_CapInitThreadVSpace;

void premain(seL4_BootInfo *bi) {
    ERRX_START;

    uint32_t image_size = (bi->userImageFrames.end - bi->userImageFrames.start) * PAGE_SIZE;
    // make sure we also don't allocate over the ipc buffer
    assert(&__executable_start + image_size == (void *) bi->ipcBuffer);
    assert(&__executable_start + image_size + 0x1000 == (void *) bi);
    image_size += PAGE_SIZE;
    mem_vspace_setup(image_size);

    if (!cslot_setup(seL4_CapInitThreadCNode, bi->empty.start, bi->empty.end)) {
        ERRX_TRACEBACK;
        fail("end");
    }

    if (!untyped_add_boot_memory(bi)) {
        ERRX_TRACEBACK;
        fail("end");
    }

    if (!mem_fx_init()) {
        ERRX_TRACEBACK;
        fail("end");
    }

    ERRX_START;

    if (!main()) {
        ERRX_TRACEBACK;
        fail("end");
    } else {
        ERRX_START;
    }

#ifdef SEL4_DEBUG_KERNEL
    seL4_DebugHalt();
#endif
    while (1) {
        // do nothing
    }
}
