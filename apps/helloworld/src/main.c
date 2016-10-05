#include <sel4/sel4.h>
#include <bedrock/debug.h>
#include <bedrock/errx.h>
#include <resource/mem_vspace.h>
#include <resource/cslot.h>
#include <elfloader/elfcontext.h>
#include <resource/mem_fx.h>

extern char __executable_start, _end;
// referenced from mem_page.c
seL4_CPtr current_vspace = ecap_PD;

bool main(void) {
    debug_println("hello, world!");
    return true;
}

void premain(seL4_IPCBuffer *buffer) {
    ERRX_START;

    mem_vspace_setup(&_end - &__executable_start);

    if (!cslot_setup(ecap_CNode, ecap_StartFree, ecap_EndFree)) {
        ERRX_TRACEBACK;
        fail("end");
    }

    // TODO: an alternative to untyped that doesn't expect preavailability of memory

    /*if (!mem_fx_init()) {
        ERRX_TRACEBACK;
        fail("end");
    }*/

    ERRX_START;

    if (!main()) {
        ERRX_TRACEBACK;
        fail("end");
    } else {
        ERRX_START;
    }

    while (1) {
        // do nothing
    }
}
