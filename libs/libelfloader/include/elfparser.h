#ifndef LIB_ELFLOADER_ELFPARSER_H
#define LIB_ELFLOADER_ELFPARSER_H

#include <sel4/errors.h>
#include <bedrock/kmem.h>
#include <bedrock/types.h>

// both used in API for elfparser_remap_cb and internally in the elf format
#define ELF_MEM_EXECUTABLE 1
#define ELF_MEM_WRITABLE 2
#define ELF_MEM_READABLE 4
#define ELF_MEM_FLAGS (ELF_MEM_EXECUTABLE | ELF_MEM_WRITABLE | ELF_MEM_READABLE)

typedef seL4_Error (*elfparser_remap_cb)(void *cookie, void *virt_target, uint8_t access_flags);

// the user must provide a PAGE_SIZE-sized page buffer, and allow using it to access PAGE_SIZE-aligned pages in the target vspace
// once elfparser_remap_cb is called, the memory should be pointing to the correct location (virt_target) and writeable.
// the access_flags value should be used AFTER the loading is complete.
extern seL4_Error elfparser_load(void *elf, size_t file_size, elfparser_remap_cb remapper, void *cookie,
                                 void *page_buffer);

#endif //LIB_ELFLOADER_ELFPARSER_H
