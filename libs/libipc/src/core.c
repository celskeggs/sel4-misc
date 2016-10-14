#include <bedrock/bedrock.h>
#include <ipc/core.h>
#include <resource/cslot.h>

bool perform_ipc(seL4_CPtr ep, seL4_CPtr cap_in, seL4_CPtr cap_out, uint32_t tag, void *params, size_t param_len,
                 void *response, size_t response_len) {
    ERRX_START;
    if (params == NULL || response == NULL || ep == seL4_CapNull) {
        ERRX_RAISE_GENERIC(GERR_NULL_VALUE);
        return false;
    }
    if ((param_len & 3) != 0 || (response_len & 3) != 0) {
        ERRX_RAISE_GENERIC(GERR_UNSATISFIED_CONSTRAINT);
        return false;
    }
    if (param_len > seL4_MsgMaxLength || response_len > seL4_MsgMaxLength) {
        ERRX_RAISE_GENERIC(GERR_REQUEST_TOO_LARGE);
        return false;
    }
    seL4_CPtr cap_out_false;
    if (cap_out == seL4_CapNull) {
        cap_out_false = cslot_alloc();
        if (cap_out_false == seL4_CapNull) {
            ERRX_TRACEPOINT;
            return false;
        }
        if (!cslot_set_receive_path(cap_out_false)) {
            ERRX_TRACEPOINT;
            return false;
        }
    } else {
        cap_out_false = seL4_CapNull;
        if (!cslot_set_receive_path(cap_out)) { // TODO: reset these to something that can't clobber afterward
            ERRX_TRACEPOINT;
            return false;
        }
    }
    uint32_t *param32 = (uint32_t *) params;
    for (uint32_t i = 0; i < (param_len >> 2); i++) {
        seL4_SetMR(i, param32[i]);
    }
    if (cap_in != seL4_CapNull) {
        seL4_SetCap(0, cap_in);
    }
    seL4_MessageInfo_t resp = seL4_Call(ep, seL4_MessageInfo_new(tag, 0, cap_in == seL4_CapNull ? 0 : 1, param_len));
    uint32_t err = seL4_MessageInfo_get_label(resp);
    uint32_t caps = seL4_MessageInfo_get_extraCaps(resp);
    uint32_t length = seL4_MessageInfo_get_length(resp);
    bool out = false;
    if (err == 0) { // success!
        uint32_t *resp32 = (uint32_t *) response;
        if (length != response_len) {
            ERRX_RAISE_GENERIC(GERR_DATA_SPILLED);
        } else {
            if ((cap_out != seL4_CapNull) != (caps != 0)) {
                DEBUG("data spill occurred");
                debug_printhex(cap_out);
                debug_printhex(caps);
                ERRX_RAISE_GENERIC(GERR_DATA_SPILLED);
            } else {
                for (uint32_t i = 0; i < (response_len >> 2); i++) {
                    resp32[i] = seL4_GetMR(i);
                }
                out = true;
            }
        }
    } else if (err > 0 && err < IPC_ERROR_GERR_BASE) { // seL4-specified error.
        ERRX_RAISE_SEL4(err);
    } else { // GERR error
        ERRX_RAISE_GENERIC(err - IPC_ERROR_GERR_BASE);
    }
    if (cap_out_false != seL4_CapNull) {
        cslot_free(cap_out_false);
    }
    return out;
}
