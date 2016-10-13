#ifndef LIB_RESOURCE_OBJECT_H
#define LIB_RESOURCE_OBJECT_H

#include <sel4/sel4.h>
#include <bedrock/errx.h> // this module uses errx

#define CNODE_4K_BITS (BITS_4KIB - 4)

seL4_CPtr object_alloc_endpoint();
seL4_CPtr object_alloc_notification();

#endif //LIB_RESOURCE_OBJECT_H
