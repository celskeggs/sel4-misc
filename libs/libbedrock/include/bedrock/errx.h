#ifndef LIB_BEDROCK_ERRNO_H
#define LIB_BEDROCK_ERRNO_H

#include <sel4/errors.h>
#include "types.h"
#include "assert.h"

#define ERRX_DEFAULT_DESC_BUF_LEN 256

// description_len includes the null terminator
typedef void (*errx_type)(uint64_t extra, char *description_out, size_t description_len);
typedef void ERRX_VOID;

extern struct errx_status {
    errx_type type;
    uint64_t extra;
} errx;

enum errx_generics {
    GERR_NONE=0,
    GERR_MEMORY_POOL_EXHAUSTED,
    GERR_INVALID_STATE,
    GERR_DATA_SPILLED
};
#define _ERRX_GENERIC_STRINGS { "OK", "Memory Pool Exhausted", "Invalid State for Operation", "Data Spilled and Lost" }

extern void errx_type_none(uint64_t, char *, size_t);

extern void errx_type_sel4(uint64_t, char *, size_t);

extern void errx_type_generic(uint64_t, char *, size_t);

#define ERRX_START assert(errx.type == errx_type_none)
#define ERRX_IFERR if (errx.type != errx_type_none)
#define ERRX_DISPLAY(dfunc) { char _errx_local_buff[ERRX_DEFAULT_DESC_BUF_LEN]; errx.type(errx.extra, _errx_local_buff, ERRX_DEFAULT_DESC_BUF_LEN); dfunc(_errx_local_buff); }
#define ERRX_CONSUME { errx.type = errx_type_none; }
#define ERRX_ASSERT { ERRX_IFERR { ERRX_DISPLAY(fail); } }

#define ERRX_RAISE(typ, extr) { ERRX_START; errx.type = typ; errx.extra = extr; }
#define ERRX_RAISE_SEL4(err_code) { ERRX_RAISE(errx_type_sel4, err_code); }
#define ERRX_RAISE_GENERIC(err_code) { ERRX_RAISE(errx_type_generic, err_code); }

#endif //LIB_BEDROCK_ERRNO_H
