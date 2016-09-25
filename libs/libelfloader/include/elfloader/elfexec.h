#ifndef LIB_ELFLOADER_ELFEXEC_H
#define LIB_ELFLOADER_ELFEXEC_H

#include <bedrock/types.h>
#include <resource/untyped.h>

struct elfexec {
    struct pagedir *pd;
    untyped_4k_ref tcb;
    untyped_4k_ref cspace;
    untyped_4k_ref page_directory;
};

bool elfexec_init(void *elf, size_t file_size, struct elfexec *holder, seL4_CPtr fault_ep, uint8_t priority,
                  seL4_CPtr io_ep, uint32_t io_badge);

void elfexec_destroy(struct elfexec *holder);

bool elfexec_start(struct elfexec *holder);

void elfexec_stop(struct elfexec *holder);

#endif //LIB_ELFLOADER_ELFEXEC_H