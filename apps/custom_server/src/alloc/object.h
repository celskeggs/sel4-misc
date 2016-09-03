#ifndef SEL4_MISC_OBJECT_H
#define SEL4_MISC_OBJECT_H

#include <sel4/sel4.h>
#include "../basic.h"

// minimum: 64 entries: 10 size bits
seL4_CNode object_alloc_cnode(uint8_t size_bits);
seL4_CPtr object_alloc_endpoint();
seL4_CPtr object_alloc_notification();
seL4_IA32_Page object_alloc_page();
seL4_IA32_Page object_alloc_page_large();
seL4_IA32_PageTable object_alloc_page_table();

#endif //SEL4_MISC_OBJECT_H
