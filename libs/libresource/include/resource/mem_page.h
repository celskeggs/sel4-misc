#ifndef LIB_RESOURCE_MEM_PAGE_H
#define LIB_RESOURCE_MEM_PAGE_H

#include <bedrock/bedrock.h>
#include <bedrock/errx.h> // this module uses errx TODO: search for seL4_Error, seL4_NoError everywhere and find bad uses
#include "untyped.h"

// NOTE: preserving the contents of this struct is only necessary if you plan to free it later!
// certain bookkeeping metadata might make sense to be permanently allocated, though be careful about that.
struct mem_page_cookie {
    void *unref_addr;
    untyped_4k_ref ref;
    seL4_CPtr mapped;
};

// TODO: support larger pages and use them in mem_arena
bool mem_page_map(void *page, struct mem_page_cookie *cookie);
bool mem_page_shared_map(void *addr, seL4_IA32_Page page, struct mem_page_cookie *cookie);
void mem_page_free(struct mem_page_cookie *cookie);

// checks to see if the cookie contains a valid page reference. when zeroed (the default), it does not.
bool mem_page_valid(struct mem_page_cookie *cookie);

#endif //LIB_RESOURCE_MEM_PAGE_H
