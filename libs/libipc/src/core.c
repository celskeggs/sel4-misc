#include <bedrock/bedrock.h>
#include <ipc/ipc.h>

bool perform_ipc(seL4_CPtr ep, uint32_t tag, void *params, size_t param_len, void *response, size_t response_len) {
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
    uint32_t *param32 = (uint32_t *) params;
    for (uint32_t i = 0; i < (param_len >> 2); i++) {
        seL4_SetMR(i, param32[i]);
    }
    seL4_MessageInfo_t resp = seL4_Call(ep, seL4_MessageInfo_new(tag, 0, 0, param_len));
    int32_t err = seL4_MessageInfo_get_label(resp);
    uint32_t length = seL4_MessageInfo_get_length(resp);
    if (err == 0) { // success!
        uint32_t *resp32 = (uint32_t *) response;
        if (length != response_len) {
            ERRX_RAISE_GENERIC(GERR_DATA_SPILLED);
            return false;
        }
        for (uint32_t i = 0; i < (response_len >> 2); i++) {
            resp32[i] = seL4_GetMR(i);
        }
        return true;
    } else if (err > 0) { // seL4-specified error.
        ERRX_RAISE_SEL4(err);
        return false;
    } else { // GERR error
        ERRX_RAISE_GENERIC(-err);
        return false;
    }
}
