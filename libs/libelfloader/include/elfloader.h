#ifndef LIB_ELFLOADER_ELFLOADER_H
#define LIB_ELFLOADER_ELFLOADER_H

#include <bedrock/bedrock.h>
#include <resource/untyped.h>

struct pagetable {
    untyped_4k_ref pt;
    untyped_4k_ref pages[PAGE_COUNT_PER_TABLE];
    uint8_t page_accesses[PAGE_COUNT_PER_TABLE];
};

struct pagedir {
    seL4_IA32_PageDirectory pd;
    struct pagetable *pts[PAGE_TABLE_COUNT];
};

struct pagedir *elfloader_load(void *elf, size_t file_size, seL4_IA32_PageDirectory page_dir, seL4_CPtr spare_cptr);

#endif //LIB_ELFLOADER_ELFLOADER_H
