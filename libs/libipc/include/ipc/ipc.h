#ifndef LIB_IPC_IPC_H
#define LIB_IPC_IPC_H

#include "core.h"

enum ipc_tag {
    IPC_ping = 256,
    IPC_init_halt,
    IPC_init_alloc,
    IPC_init_free,
};

DECLARE_IPC(ping, int32_t value;, int32_t value_neg;)
DECLARE_IPC(init_halt, ,)
DECLARE_IPC_CO(init_alloc, uint32_t object_type;, uint32_t cookie;)

#endif //LIB_IPC_IPC_H
