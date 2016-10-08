#include <bedrock/debug.h>
#include <init/init.h>
#include <ipc/ipc.h>

bool main(void) {
    debug_println("hello, sandbox world!");
    struct ipc_out_ping out;
    if (!ipc_ping(ecap_IOEP, &(struct ipc_in_ping) {.value = 102}, &out)) {
        ERRX_TRACEPOINT;
        return false;
    }
    assert(out.value_neg == ~102);
    DEBUG("received!");
    return true;
}
