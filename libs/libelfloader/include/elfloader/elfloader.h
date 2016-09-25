#ifndef LIB_ELFLOADER_ELFLOADER_H
#define LIB_ELFLOADER_ELFLOADER_H

#include <bedrock/bedrock.h>
#include <resource/untyped.h>
#include <bedrock/errx.h> // this module uses errx

struct pagetable {
    untyped_4k_ref pt;
    untyped_4k_ref pages[PAGE_COUNT_PER_TABLE];
    uint8_t page_accesses[PAGE_COUNT_PER_TABLE];
};

struct pagedir {
    seL4_IA32_PageDirectory pd;
    struct pagetable *pts[PAGE_TABLE_COUNT];
    uintptr_t entry_position;
};

struct pagedir *elfloader_load(void *elf, size_t file_size, seL4_IA32_PageDirectory page_dir);
seL4_IA32_Page elfloader_get_page(struct pagedir *pd, void *virtual_address, uint8_t access_flags, bool exclusive);
void elfloader_unload(struct pagedir *pd);

#endif //LIB_ELFLOADER_ELFLOADER_H
