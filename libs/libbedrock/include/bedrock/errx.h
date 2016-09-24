#ifndef LIB_BEDROCK_ERRNO_H
#define LIB_BEDROCK_ERRNO_H

#include <sel4/errors.h>
#include "types.h"
#include "assert.h"
#include "debug.h"

#define ERRX_DEFAULT_DESC_BUF_LEN 1024
#define ERRX_TRACEBACK_DEPTH 64

// description_len includes the null terminator. errx_status is a struct errx_status *, but weird C typing rules prevent
// us from stating that directly.
typedef void (*errx_type)(void *errx_status, char *description_out, size_t description_len);

extern struct errx_status {
    errx_type type;
    uint64_t extra;
    const char *traceback_elems[ERRX_TRACEBACK_DEPTH];
    uint32_t traceback_next;
} errx;

enum errx_generics {
    GERR_NONE = 0,
    GERR_MEMORY_POOL_EXHAUSTED,
    GERR_INVALID_STATE,
    GERR_DATA_SPILLED,
    GERR_INVALID_MAGIC,
    GERR_UNSUPPORTED_OPTION,
    GERR_MALFORMED_DATA,
    GERR_REQUEST_TOO_LARGE,
    GERR_OUT_OF_RANGE,
};
#define _ERRX_GENERIC_STRINGS { "OK", "Memory Pool Exhausted", "Invalid State for Operation", "Data Spilled and Lost", \
                                "Invalid Magic Number", "Unsupported Option", "Malformed Data", "Request Too Large", \
                                "Parameter Out of Range" }

extern void errx_type_none(void *errx_status, char *description_out, size_t description_len);

extern void errx_type_sel4(void *errx_status, char *description_out, size_t description_len);

extern void errx_type_generic(void *errx_status, char *description_out, size_t description_len);

#define ERRX_START assert(errx.type == errx_type_none)
#define ERRX_IFERR if (errx.type != errx_type_none)
#define ERRX_TRACEBACK { char _errx_local_buff[ERRX_DEFAULT_DESC_BUF_LEN]; ERRX_TRACEPOINT; errx.type(&errx, _errx_local_buff, ERRX_DEFAULT_DESC_BUF_LEN); debug_println(_errx_local_buff); }
#define ERRX_CONSUME { errx.type = errx_type_none; errx.traceback_next = 0; }
#define ERRX_ASSERT { ERRX_IFERR { ERRX_DISPLAY(fail); } }
#define _ERRX_TRACEPOINT(file, line) {\
    assert(errx.type != errx_type_none); \
    if (errx.traceback_next >= ERRX_TRACEBACK_DEPTH) { \
        errx.traceback_elems[ERRX_TRACEBACK_DEPTH - 1] = "... traceback elised ..."; \
    } else { \
        errx.traceback_elems[errx.traceback_next++] = file ":" _fail_tostring(line); \
    } \
}
#define ERRX_TRACEPOINT _ERRX_TRACEPOINT(__FILE__, __LINE__)

extern void errx_concat_traceback(struct errx_status *, char *, size_t);

#define ERRX_RAISE(typ, extr) { ERRX_START; errx.type = typ; errx.extra = extr; errx.traceback_next = 0; ERRX_TRACEPOINT; }
#define ERRX_RAISE_SEL4(err_code) { ERRX_RAISE(errx_type_sel4, err_code); }
#define ERRX_RAISE_GENERIC(err_code) { ERRX_RAISE(errx_type_generic, err_code); }

#endif //LIB_BEDROCK_ERRNO_H
