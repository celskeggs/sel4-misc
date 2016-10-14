#include <bedrock/debug.h>
#include <init/init.h>
#include <ipc/ipc.h>
#include <resource/cslot.h>

bool main(void) {
    debug_println("hello, sandbox world!");
    struct ipc_out_ping out;
    if (!ipc_ping(ecap_IOEP, &(struct ipc_in_ping) {.value = 102}, &out)) {
        ERRX_TRACEPOINT;
        return false;
    }
    assert(out.value_neg == ~102);
    DEBUG("received!");
    seL4_CPtr cap_out = cslot_alloc();
    if (cap_out == seL4_CapNull) {
        ERRX_TRACEPOINT;
        return false;
    }
    struct ipc_out_alloc out4k;
    if (!ipc_alloc(ecap_IOEP, cap_out, &(struct ipc_in_alloc) {.object_type = seL4_IA32_4K}, &out4k)) {
        ERRX_TRACEPOINT;
        return false;
    }
    debug_printhex(out4k.cookie);
    DEBUG("received 2!");
    int err = seL4_IA32_Page_Map(cap_out, ecap_PD, 0xFFFF0000, seL4_AllRights, seL4_IA32_Default_VMAttributes);
    if (err != seL4_NoError) {
        DEBUG("expected error hit!");
        ERRX_RAISE_SEL4(err);
        return false;
    }
    DEBUG("expected error not hit!");
    return true;
}
