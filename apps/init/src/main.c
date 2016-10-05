#include <sel4/sel4.h>
#include "serial.h"
#include <elfloader/elfexec.h>
#include <resource/cslot.h>
#include <resource/untyped.h>
#include <resource/mem_page.h>
#include <resource/mem_vspace.h>
#include <resource/mem_fx.h>

extern char *image_helloworld;
extern char *image_helloworld_end;

bool main(void) {
    const char *source = "Hello, serial world Nth!\n";
    char *buf = mem_fx_alloc(64);
    if (buf == NULL) {
        return false;
    }
    memcpy(buf, source, strlen(source));
    if (!serial_write(buf, (size_t) strlen(buf))) {
        return false;
    }
    // serial_wait_ready();
    struct elfexec context;
    if (!elfexec_init(image_helloworld, image_helloworld_end - image_helloworld, &context, seL4_CapNull, 255, seL4_CapNull, 0)) {
        return false;
    }
    if (!elfexec_start(&context)) {
        return false;
    }
    DEBUG("STARTED");
    for (int i = 0; i < 100000000; i++) {
        asm("nop");
    }
    DEBUG("STOPPED");
    return true;
}

extern char __executable_start;
// referenced from mem_page.c
seL4_CPtr current_vspace = seL4_CapInitThreadVSpace;

void premain(seL4_BootInfo *bi) {
    ERRX_START;

    mem_vspace_setup((bi->userImageFrames.end - bi->userImageFrames.start) * PAGE_SIZE, bi->ipcBuffer, bi);
    if (!cslot_setup(seL4_CapInitThreadCNode, bi->empty.start, bi->empty.end)) {
        ERRX_TRACEBACK;
        fail("end");
    }

    if (!untyped_add_boot_memory(bi)) {
        ERRX_TRACEBACK;
        fail("end");
    }

    if (!serial_init(seL4_CapIOPort, seL4_CapIRQControl, NULL)) {
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
