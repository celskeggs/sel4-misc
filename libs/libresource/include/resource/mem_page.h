#ifndef SEL4_MISC_MEM_PAGE_H
#define SEL4_MISC_MEM_PAGE_H

#include <bedrock/bedrock.h>
#include "untyped.h"

struct mem_page_cookie {
    void *unref_addr;
    untyped_4k_ref ref;
};

// TODO: support larger pages and use them in mem_arena
seL4_Error mem_page_map(void *page, struct mem_page_cookie *cookie);
seL4_Error mem_page_shared_map(void *addr, seL4_IA32_Page page);
void mem_page_shared_free(void *addr, seL4_IA32_Page page);
void mem_page_free(struct mem_page_cookie *data);

#endif //SEL4_MISC_MEM_PAGE_H
