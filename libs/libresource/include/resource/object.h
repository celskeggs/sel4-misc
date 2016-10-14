#ifndef LIB_RESOURCE_OBJECT_H
#define LIB_RESOURCE_OBJECT_H

#include <sel4/sel4.h>
#include <bedrock/errx.h> // this module uses errx

#define CNODE_4K_BITS (BITS_4KIB - 4)
typedef void *object_token;

// NOTE: these are not provided by libresource! they MUST be provided by someone else if they are to be used!

object_token object_alloc(uint32_t object_type);

seL4_CPtr object_cap(object_token token);

void object_free_token(object_token token);

#endif //LIB_RESOURCE_OBJECT_H
