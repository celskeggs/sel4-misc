#include <bedrock/bedrock.h>
#include <elfloader/ctxexec.h>
#include <elfloader/elfcontext.h>
#include <ipc/ipc.h>

uint32_t exec_init(seL4_CPtr cspace, uint32_t ipc_address, uint8_t priority, uint32_t entry_vector) {
    ERRX_START;
    struct ipc_out_proc_init out;
    if (!ipc_proc_init(ecap_IOEP, cspace,
                       &(struct ipc_in_proc_init) {.entry_vector=entry_vector, .ipc_addr=ipc_address, .pri=priority},
                       &out)) {
        ERRX_TRACEPOINT;
        return 0;
    }
    assert(out.cookie != 0);
    return out.cookie;
}

bool exec_start(uint32_t cookie_r) {
    ERRX_START;
    assert(cookie_r != 0);
    if (!ipc_proc_start(ecap_IOEP, &(struct ipc_in_proc_start) {.cookie=cookie_r}, NULL)) {
        ERRX_TRACEPOINT;
        return false;
    }
    return true;
}

void exec_stop(uint32_t cookie_r) {
    ERRX_SAVE();
    assert(cookie_r != 0);
    assert(ipc_proc_stop(ecap_IOEP, &(struct ipc_in_proc_stop) {.cookie=cookie_r}, NULL));
    ERRX_LOAD();
}

void exec_destroy(uint32_t cookie_r) {
    ERRX_SAVE();
    assert(cookie_r != 0);
    assert(ipc_proc_destroy(ecap_IOEP, &(struct ipc_in_proc_destroy) {.cookie=cookie_r}, NULL));
    ERRX_LOAD();
}
