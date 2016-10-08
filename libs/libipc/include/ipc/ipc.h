#ifndef LIB_IPC_IPC_H
#define LIB_IPC_IPC_H

#include "core.h"

enum ipc_tag {
    IPC_ping = 256,
    IPC_init_halt,
    IPC_init_alloc_4k,
    IPC_init_free_4k,
    IPC_init_alloc_4m, // TODO
    IPC_init_free_4m, // TODO
};

DECLARE_IPC(ping, int32_t value;, int32_t value_neg;)
DECLARE_IPC(init_halt, ,)
DECLARE_IPC_CO(init_alloc_4k, , uint32_t cookie;)

#endif //LIB_IPC_IPC_H
