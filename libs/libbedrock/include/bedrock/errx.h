#ifndef LIB_BEDROCK_ERRNO_H
#define LIB_BEDROCK_ERRNO_H

#include <sel4/errors.h>
#include "types.h"
#include "assert.h"

#define ERRX_DEFAULT_DESC_BUF_LEN 256

// description_len includes the null terminator
typedef void (*errx_type)(uint64_t extra, char *description_out, size_t description_len);

extern struct errx_status {
    errx_type type;
    uint64_t extra;
} errx;

extern void errx_type_none(uint64_t, char *, size_t);

extern void errx_type_sel4(uint64_t, char *, size_t);

#define ERRX_START assert(errx.type == errx_type_none)
#define ERRX_IFERR if (errx.type != errx_type_none)
#define ERRX_DISPLAY(dfunc) { char _errx_local_buff[ERRX_DEFAULT_DESC_BUF_LEN]; errx.type(errx.extra, _errx_local_buff, ERRX_DEFAULT_DESC_BUF_LEN); dfunc(_errx_local_buff); }
#define ERRX_CONSUME { errx.type = errx_type_none; }
#define ERRX_ASSERT { ERRX_IFERR { ERRX_DISPLAY(fail); } }

#define ERRX_RAISE(type, extra) { ERRX_START; errx.type = type; errx.extra = extra; }
#define ERRX_RAISE_SEL4(err_code) { ERRX_START; errx.type = errx_type_sel4; errx.extra = err_code; }

#define ERRX_CHECK_SEL4(err_code) if ((errx.type = errx_type_none) && (errx.extra = err_code) != seL4_NoError && (errx.type = errx_type_sel4))

#endif //LIB_BEDROCK_ERRNO_H
