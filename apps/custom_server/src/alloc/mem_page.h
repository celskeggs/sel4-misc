#ifndef SEL4_MISC_MEM_PAGE_H
#define SEL4_MISC_MEM_PAGE_H

#include <sel4/sel4.h>
#include "untyped.h"

#define PAGE_SIZE (1U << seL4_PageBits)
#define PAGE_TABLE_SIZE (1U << seL4_LargePageBits)
#define KERNEL_BASE_VADDR 0xE0000000ULL
#define PAGE_TABLE_COUNT (KERNEL_BASE_VADDR / PAGE_TABLE_SIZE)
#define PAGE_COUNT_PER_TABLE (PAGE_TABLE_SIZE / PAGE_SIZE)
seL4_CompileTimeAssert(PAGE_SIZE == 4096);
seL4_CompileTimeAssert(PAGE_TABLE_SIZE == 4194304);
seL4_CompileTimeAssert(PAGE_TABLE_COUNT == 896);
seL4_CompileTimeAssert(PAGE_COUNT_PER_TABLE == 1024);

struct mem_page_cookie {
    void *unref_addr;
    untyped_ref ref;
};

// TODO: support larger pages and use them in mem_arena
seL4_Error mem_page_map(void *page, struct mem_page_cookie *cookie);
void mem_page_free(struct mem_page_cookie *data);

#endif //SEL4_MISC_MEM_PAGE_H
