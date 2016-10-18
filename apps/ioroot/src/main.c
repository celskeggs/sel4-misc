#include <bedrock/debug.h>
#include <init/init.h>
#include <ipc/ipc.h>

bool main(void) {
    seL4_CPtr registrar = cslot_alloc();
    if (registrar == seL4_CapNull) {
        ERRX_TRACEPOINT;
        return false;
    }
    if (!ipc_sandbox_get_registrar(ecap_IOEP, registrar, NULL, NULL)) {
        ERRX_TRACEPOINT;
        return false;
    }
    return true;
}
