#ifndef LIB_RESOURCE_OBJECT_H
#define LIB_RESOURCE_OBJECT_H

#include <sel4/sel4.h>
#include <bedrock/errx.h> // this module uses errx

seL4_CPtr object_alloc_endpoint();
seL4_CPtr object_alloc_notification();
seL4_IA32_Page object_alloc_page();
seL4_IA32_PageTable object_alloc_page_table();

#endif //LIB_RESOURCE_OBJECT_H
