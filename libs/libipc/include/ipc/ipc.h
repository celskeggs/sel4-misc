#ifndef LIB_IPC_IPC_H
#define LIB_IPC_IPC_H

#include "core.h"

enum ipc_tag {
    IPC_ping = 256,
    IPC_init_halt,
    IPC_alloc,
    IPC_free,
    IPC_sandbox_ready,
};

DECLARE_IPC(ping, int32_t value;, int32_t value_neg;)
DECLARE_IPC(init_halt, ,)
DECLARE_IPC_CO(alloc, uint32_t object_type;, uint32_t cookie;)
DECLARE_IPC(free, uint32_t cookie;,)
DECLARE_IPC_CI(sandbox_ready, ,)

#endif //LIB_IPC_IPC_H
