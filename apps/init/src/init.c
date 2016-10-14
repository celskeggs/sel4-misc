#include <bedrock/bedrock.h>
#include <resource/mem_vspace.h>
#include <resource/cslot.h>
#include <resource/mem_fx.h>
#include "untyped.h"

extern bool main(void);

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
