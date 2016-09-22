#include <sel4/sel4.h>
#include "serial.h"
#include <resource/cslot_ao.h>
#include <resource/untyped.h>
#include <resource/mem_page.h>
#include <resource/mem_vspace.h>
#include <resource/mem_fx.h>

bool main(void) {
    const char *source = "Hello, serial world Nth!\n";
    char *buf = mem_fx_alloc(64);
    if (buf == NULL) {
        return false;
    }
    char *dest = buf;
    do {
        *dest++ = *source;
    } while (*source++);
    if (!serial_write(buf, (size_t) strlen(buf))) {
        return false;
    }
    serial_wait_ready();
    int i = 1000000000;
    while (i-- > 0) {
        asm("nop");
    }
    return serial_write(buf, (size_t) strlen(buf));
}

extern char __executable_start;
// referenced from mem_page.c
seL4_CPtr current_vspace = seL4_CapInitThreadVSpace;

void premain(seL4_BootInfo *bi) {
    ERRX_START;

    mem_vspace_setup((bi->userImageFrames.end - bi->userImageFrames.start) * PAGE_SIZE, bi->ipcBuffer, bi);
    cslot_ao_setup(seL4_CapInitThreadCNode, bi->empty.start, bi->empty.end);
    assert(untyped_add_boot_memory(bi));

    assert(serial_init(seL4_CapIOPort, seL4_CapIRQControl, NULL));

    if (!mem_fx_init()) {
        ERRX_DISPLAY(fail)
    }

    ERRX_START;

    if (!main()) {
        ERRX_DISPLAY(fail);
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
