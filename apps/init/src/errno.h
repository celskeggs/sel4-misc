#ifndef SEL4_MISC_ERRNO_H
#define SEL4_MISC_ERRNO_H

#include <sel4/errors.h>

const char *err_to_string(seL4_Error err);
static inline const char *erri_to_string(int err) {
    return err_to_string((seL4_Error) err);
}

#endif //SEL4_MISC_ERRNO_H
