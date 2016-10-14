#ifndef LIB_ELFLOADER_ELFEXEC_H
#define LIB_ELFLOADER_ELFEXEC_H

#include <bedrock/types.h>
#include <resource/object.h>

struct elfexec {
    struct pagedir *pd;
    object_token tcb;
    object_token cspace;
    object_token page_directory;
};

bool elfexec_init(void *elf, size_t file_size, struct elfexec *holder, seL4_CPtr fault_ep, uint8_t priority,
                  seL4_CPtr io_ep);

void elfexec_destroy(struct elfexec *holder);

bool elfexec_start(struct elfexec *holder);

void elfexec_stop(struct elfexec *holder);

#endif //LIB_ELFLOADER_ELFEXEC_H