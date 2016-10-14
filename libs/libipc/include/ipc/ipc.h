#ifndef LIB_IPC_IPC_H
#define LIB_IPC_IPC_H

#include "core.h"

enum ipc_tag {
    IPC_ping = 256,
    IPC_init_halt,
    IPC_alloc,
    IPC_free,
    IPC_sandbox_ready,
    IPC_proc_init,
    IPC_proc_destroy,
    IPC_proc_start,
    IPC_proc_stop,
};

DECLARE_IPC(ping, int32_t value;, int32_t value_neg;)
DECLARE_IPC(init_halt, ,)
DECLARE_IPC_CO(alloc, uint32_t object_type;, uint32_t cookie;)
DECLARE_IPC(free, uint32_t cookie;,)
DECLARE_IPC_CI(sandbox_ready, ,) // takes cap to export
DECLARE_IPC_CI(proc_init, uint32_t ipc_addr;
        uint8_t pri;
        uint32_t entry_vector;,
               uint32_t cookie;) // takes cspace cap
DECLARE_IPC(proc_destroy, uint32_t cookie;,)
DECLARE_IPC(proc_start, uint32_t cookie;,)
DECLARE_IPC(proc_stop, uint32_t cookie;,)


#endif //LIB_IPC_IPC_H
