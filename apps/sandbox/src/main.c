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
    struct ipc_out_init_alloc_4k out4k;
    if (!ipc_init_alloc_4k(ecap_IOEP, cap_out, &(struct ipc_in_init_alloc_4k) {}, &out4k)) {
        ERRX_TRACEPOINT;
        return false;
    }
    debug_printhex(out4k.cookie);
    DEBUG("received 2!");
    seL4_CPtr cap_2 = cslot_alloc();
    if (cap_2 == seL4_CapNull) {
        ERRX_TRACEPOINT;
        return false;
    }
    DEBUG("retyping...");
    if (!cslot_retype(cap_out, seL4_IA32_4K, 0, 0, cap_2, 1)) {
        ERRX_TRACEPOINT;
        return false;
    }
    DEBUG("retyped!");
    int err = seL4_IA32_Page_Map(cap_2, ecap_PD, 0xFFFF0000, seL4_AllRights, seL4_IA32_Default_VMAttributes);
    if (err != seL4_NoError) {
        DEBUG("expected error hit!");
        ERRX_RAISE_SEL4(err);
        return false;
    }
    DEBUG("expected error not hit!");
    return true;
}
