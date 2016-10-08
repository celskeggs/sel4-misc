#ifndef LIB_IPC_CORE_H
#define LIB_IPC_CORE_H

#include <sel4/sel4.h>
#include <bedrock/bedrock.h>
#include <bedrock/errx.h>

bool perform_ipc(seL4_CPtr ep, uint32_t tag, void *in, size_t in_len, void *out, size_t out_len);

#define DECLARE_IPC(ipcname, fields_in, fields_out) \
struct ipc_in_##ipcname { \
    fields_in \
}; \
struct ipc_out_##ipcname { \
    fields_out \
}; \
static inline bool ipc_##ipcname(seL4_CPtr ep, struct ipc_in_##ipcname *in, struct ipc_out_##ipcname *out) {\
    return perform_ipc(ep, IPC_##ipcname, in, sizeof(*in), out, sizeof(*out));\
} \
bool ipc_handle_##ipcname(uint32_t sender, struct ipc_in_##ipcname *in, struct ipc_out_##ipcname *out); \
seL4_CompileTimeAssert(sizeof(struct ipc_in_##ipcname) <= seL4_MsgMaxLength); \
seL4_CompileTimeAssert(sizeof(struct ipc_out_##ipcname) <= seL4_MsgMaxLength); \
seL4_CompileTimeAssert((sizeof(struct ipc_in_##ipcname) & 3) == 0); \
seL4_CompileTimeAssert((sizeof(struct ipc_out_##ipcname) & 3) == 0);

#define SERVER_FOR(ipcname) \
    case IPC_##ipcname: \
        if (length != sizeof(struct ipc_in_##ipcname)) {\
            response = -GERR_UNSATISFIED_CONSTRAINT; \
        } else if (ipc_handle_##ipcname(sender, (struct ipc_in_##ipcname *) data_buffer_in, (struct ipc_out_##ipcname *) data_buffer_out)) { \
            response = 0; \
            response_len = sizeof(struct ipc_out_##ipcname); \
        } else { /* failed */ \
            ERRX_TRACEPOINT; \
            /* TODO: consider printing traceback here */ \
            if (errx.type == errx_type_sel4 && errx.extra > 0) { \
                response = (uint32_t) errx.extra; \
            } else if (errx.type == errx_type_generic && errx.extra > 0) { \
                response = (uint32_t) -errx.extra; \
            } else { \
                response = -GERR_UNKNOWN_ERROR; \
            } \
            ERRX_CONSUME; \
        } \
        break;

#define SERVER_LOOP(ep, condition, handlers) \
    while (condition) { \
        ERRX_START; \
        uint32_t data_buffer_in[seL4_MsgMaxLength >> 2]; \
        uint32_t data_buffer_out[seL4_MsgMaxLength >> 2]; \
        uint32_t sender; \
        seL4_MessageInfo_t recv = seL4_Recv(ep, &sender); \
        uint32_t label = seL4_MessageInfo_get_label(recv); \
        uint32_t length = seL4_MessageInfo_get_length(recv); \
        uint32_t response, response_len = 0; \
        if (length > seL4_MsgMaxLength) { length = seL4_MsgMaxLength; } \
        for (uint32_t i = 0; i < (length >> 2); i++) { \
            data_buffer_in[i] = seL4_GetMR(i); \
        } \
        switch (label) { \
            handlers \
        default: \
            response = -GERR_UNSUPPORTED_OPTION; \
            break; \
        } \
        ERRX_START; \
        for (uint32_t i = 0; i < (response_len >> 2); i++) { \
            seL4_SetMR(i, data_buffer_out[i]); \
        } \
        seL4_Reply(seL4_MessageInfo_new(response, 0, 0, response_len & ~3)); \
    }


#endif //LIB_IPC_CORE_H
