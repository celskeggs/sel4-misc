#ifndef LIB_BEDROCK_ERRNO_H
#define LIB_BEDROCK_ERRNO_H

#include <sel4/errors.h>

extern const char *err_to_string(seL4_Error err);

static inline const char *erri_to_string(int err) {
    return err_to_string((seL4_Error) err);
}

#endif //LIB_BEDROCK_ERRNO_H
