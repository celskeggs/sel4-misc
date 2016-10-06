#include <bedrock/debug.h>
#include <init/init.h>

bool main(void) {
    debug_println("hello, sandbox world!");
    seL4_MessageInfo_t resp = seL4_Call(ecap_IOEP, seL4_MessageInfo_new(RL_TEST, 0, 0, 0));
    DEBUG("received");
    debug_printdec(seL4_MessageInfo_get_label(resp));
    debug_printdec(seL4_MessageInfo_get_capsUnwrapped(resp));
    debug_printdec(seL4_MessageInfo_get_extraCaps(resp));
    debug_printdec(seL4_MessageInfo_get_length(resp));
    DEBUG("done");
    return true;
}
