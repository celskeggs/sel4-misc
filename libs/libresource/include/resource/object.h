#ifndef SEL4_MISC_OBJECT_H
#define SEL4_MISC_OBJECT_H

#include <sel4/sel4.h>

seL4_CPtr object_alloc_endpoint();
seL4_CPtr object_alloc_notification();
seL4_IA32_Page object_alloc_page();
seL4_IA32_PageTable object_alloc_page_table();

#endif //SEL4_MISC_OBJECT_H
