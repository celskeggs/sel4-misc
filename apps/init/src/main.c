#include <sel4/sel4.h>
#include <elfloader/elfexec.h>
#include <resource/cslot.h>
#include <resource/mem_vspace.h>
#include <resource/mem_fx.h>
#include <resource/object.h>
#include <elfloader/elfcontext.h>

extern char *image_sandbox;
extern char *image_sandbox_end;

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
    while (true) {
        seL4_MessageInfo_t recv = seL4_Recv(root_endpoint, NULL);
        enum root_label label = (enum root_label) seL4_MessageInfo_get_label(recv);
        uint32_t length = seL4_MessageInfo_get_length(recv);
        uint32_t extracaps = seL4_MessageInfo_get_extraCaps(recv);
        uint32_t capsUnwrapped = seL4_MessageInfo_get_capsUnwrapped(recv);
        uint32_t response;
        DEBUG("recv");
        debug_printdec(label);
        debug_printdec(length);
        debug_printdec(extracaps);
        debug_printdec(capsUnwrapped);
        switch (label) {
            case RL_TEST:
                DEBUG("test receive successful");
                response = 162;
                break;
            case RL_HALT:
                DEBUG("HALT");
                return true;
            default:
                DEBUG("invalid receive...");
                response = 0;
                break;
        }
        seL4_Reply(seL4_MessageInfo_new(response, 0, 0, 0));
    }
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
