#ifndef LIB_IPC_IPC_H
#define LIB_IPC_IPC_H

#include "core.h"

enum ipc_tag {
    IPC_ping = 256,
    IPC_init_halt,
    IPC_alloc,
    IPC_free,
    IPC_sandbox_set_registrar,
    IPC_sandbox_get_registrar,
    IPC_proc_init,
    IPC_proc_destroy,
    IPC_proc_start,
    IPC_proc_stop,
    IPC_registrar_iter_first,
    IPC_registrar_iter_next,
    IPC_registrar_derive,
    IPC_registrar_lookup,
    IPC_registrar_register,
    IPC_registrar_remove,
};

DECLARE_IPC(ping, int32_t value;, int32_t value_neg;)
DECLARE_IPC(init_halt, ,)
DECLARE_IPC_CO(alloc, uint32_t object_type;, uint32_t cookie;)
DECLARE_IPC(free, uint32_t cookie;,)
DECLARE_IPC_CI(sandbox_set_registrar, ,) // takes cap to export
DECLARE_IPC_CO(sandbox_get_registrar, ,) // provides registrar cap
DECLARE_IPC_CI(proc_init, uint32_t ipc_addr;
        uint8_t pri;
        uint32_t entry_vector;,
               uint32_t cookie;) // takes cspace cap
DECLARE_IPC(proc_destroy, uint32_t cookie;,)
DECLARE_IPC(proc_start, uint32_t cookie;,)
DECLARE_IPC(proc_stop, uint32_t cookie;,)
DECLARE_IPC(registrar_iter_first, , bool any;
        char name[IPC_STR_LEN];)
DECLARE_IPC(registrar_iter_next, char name[IPC_STR_LEN];,
            char name[IPC_STR_LEN];)
DECLARE_IPC_CO(registrar_derive, bool allow_write;
        char prefix[IPC_STR_LEN];,)
DECLARE_IPC_CO(registrar_lookup, char name[IPC_STR_LEN];,)
DECLARE_IPC_CI(registrar_register, char name[IPC_STR_LEN];,)
DECLARE_IPC(registrar_remove, char name[IPC_STR_LEN];,)

#endif //LIB_IPC_IPC_H
