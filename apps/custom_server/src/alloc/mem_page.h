#ifndef SEL4_MISC_MEM_PAGE_H
#define SEL4_MISC_MEM_PAGE_H

#include <sel4/sel4.h>

#define PAGE_SIZE (1 << seL4_PageBits)
seL4_CompileTimeAssert(PAGE_SIZE == 4096);

void *mem_page_alloc();
void mem_page_free(void *page);

#endif //SEL4_MISC_MEM_PAGE_H
