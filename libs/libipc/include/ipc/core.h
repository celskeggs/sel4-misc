#ifndef LIB_IPC_CORE_H
#define LIB_IPC_CORE_H

#include <sel4/sel4.h>
#include <bedrock/bedrock.h>
#include <bedrock/errx.h>

#define IPC_ERROR_GERR_BASE 256

bool perform_ipc(seL4_CPtr ep, seL4_CPtr cap_in, seL4_CPtr cap_out, uint32_t tag, void *in, size_t in_len, void *out,
                 size_t out_len);

#define _DECLARE_IPC(ipcname, cptrs, cptrsp, cptrrev, cptri, fields_in, fields_out) \
struct ipc_in_##ipcname { \
    fields_in \
}; \
struct ipc_out_##ipcname { \
    fields_out \
}; \
static inline bool ipc_##ipcname(seL4_CPtr ep cptrs, struct ipc_in_##ipcname *in, struct ipc_out_##ipcname *out) {\
    return perform_ipc(ep, cptrsp, IPC_##ipcname, in, sizeof(*in), out, sizeof(*out));\
} \
static int ipc_cap_meta_##ipcname = cptri; \
bool ipc_handle_##ipcname(uint32_t sender cptrs, struct ipc_in_##ipcname *in, struct ipc_out_##ipcname *out); \
static inline bool ipc_handle_wrap_##ipcname(uint32_t sender, seL4_CPtr cap_in, seL4_CPtr cap_out, void *in, void *out) { \
    (void) ipc_cap_meta_##ipcname; \
    (void) cap_in; \
    (void) cap_out; \
    return ipc_handle_##ipcname(sender cptrrev, (struct ipc_in_##ipcname *) in, (struct ipc_out_##ipcname *) out); \
} \
seL4_CompileTimeAssert(sizeof(struct ipc_in_##ipcname) <= seL4_MsgMaxLength); \
seL4_CompileTimeAssert(sizeof(struct ipc_out_##ipcname) <= seL4_MsgMaxLength); \
seL4_CompileTimeAssert((sizeof(struct ipc_in_##ipcname) & 3) == 0); \
seL4_CompileTimeAssert((sizeof(struct ipc_out_##ipcname) & 3) == 0);

#define __COMMA ,
#define DECLARE_IPC(ipcname, fields_in, fields_out) \
    _DECLARE_IPC(ipcname, , seL4_CapNull __COMMA seL4_CapNull, , 0, fields_in, fields_out)
#define DECLARE_IPC_CI(ipcname, fields_in, fields_out) \
    _DECLARE_IPC(ipcname, __COMMA seL4_CPtr cap_in, cap_in __COMMA seL4_CapNull, __COMMA cap_in, 1, fields_in, fields_out)
#define DECLARE_IPC_CO(ipcname, fields_in, fields_out) \
    _DECLARE_IPC(ipcname, __COMMA seL4_CPtr cap_out, seL4_CapNull __COMMA cap_out, __COMMA cap_out, 2, fields_in, fields_out)
#define DECLARE_IPC_CIO(ipcname, fields_in, fields_out) \
    _DECLARE_IPC(ipcname, __COMMA seL4_CPtr cap_in, __COMMA seL4_CPtr cap_out, cap_in __COMMA cap_out, __COMMA cap_in __COMMA cap_out, 3, fields_in, fields_out)

#define SERVER_FOR(ipcname) \
    case IPC_##ipcname: \
        if (length != sizeof(struct ipc_in_##ipcname)) { \
            response = -GERR_UNSATISFIED_CONSTRAINT; \
        } else if (caps != (ipc_cap_meta_##ipcname & 1)) { /* cap count mismatch */\
            response = -GERR_UNSATISFIED_CONSTRAINT; \
        } else if (ipc_handle_wrap_##ipcname(sender, recv_cptr, send_cptr, data_buffer_in, data_buffer_out)) { \
            response = 0; \
            response_len = sizeof(struct ipc_out_##ipcname); \
            if (ipc_cap_meta_##ipcname & 3) { \
                response_caps = 1; \
            } \
        } else { /* failed */ \
            ERRX_TRACEPOINT; \
            DEBUG("responding to IPC with error:"); \
            ERRX_TRACEBACK; \
            /* TODO: consider printing traceback here */ \
            if (errx.type == errx_type_sel4 && errx.extra > 0) { \
                response = (uint32_t) errx.extra; \
            } else if (errx.type == errx_type_generic && errx.extra > 0) { \
                response = IPC_ERROR_GERR_BASE + (uint32_t) errx.extra; \
            } else { \
                response = IPC_ERROR_GERR_BASE + GERR_UNKNOWN_ERROR; \
            } \
            ERRX_CONSUME; \
        } \
        break;

#define SERVER_LOOP(ep, condition, handlers) \
    { \
        seL4_CPtr recv_cptr = cslot_alloc(), send_cptr = cslot_alloc(); \
        assert(recv_cptr != seL4_CapNull && send_cptr != seL4_CapNull); /* todo: handle this with proper error handling */ \
        while (condition) { \
            ERRX_START; \
            uint32_t data_buffer_in[seL4_MsgMaxLength >> 2]; \
            uint32_t data_buffer_out[seL4_MsgMaxLength >> 2]; \
            uint32_t sender; \
            assert(cslot_set_receive_path(recv_cptr)); \
            seL4_MessageInfo_t recv = seL4_Recv(ep, &sender); \
            uint32_t label = seL4_MessageInfo_get_label(recv); \
            uint32_t length = seL4_MessageInfo_get_length(recv); \
            uint32_t caps = seL4_MessageInfo_get_extraCaps(recv); \
            uint32_t response, response_len = 0, response_caps = 0; \
            if (length > seL4_MsgMaxLength) { length = seL4_MsgMaxLength; } \
            for (uint32_t i = 0; i < (length >> 2); i++) { \
                data_buffer_in[i] = seL4_GetMR(i); \
            } \
            switch (label) { \
                handlers \
            default: \
                DEBUG("specified label was not found in server's domain"); \
                response = IPC_ERROR_GERR_BASE + GERR_UNSUPPORTED_OPTION; \
                break; \
            } \
            ERRX_START; \
            for (uint32_t i = 0; i < (response_len >> 2); i++) { \
                seL4_SetMR(i, data_buffer_out[i]); \
            } \
            if (response_caps) { \
                seL4_SetCap(0, send_cptr); \
            } \
            seL4_Reply(seL4_MessageInfo_new(response, 0, response_caps, response_len & ~3)); \
            assert(cslot_delete(send_cptr)); \
            assert(cslot_delete(recv_cptr)); \
        } \
        cslot_free(recv_cptr); \
        cslot_free(send_cptr); \
    }

#endif //LIB_IPC_CORE_H
