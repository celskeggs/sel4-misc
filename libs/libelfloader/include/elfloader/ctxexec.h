#ifndef LIB_ELFLOADER_CTXEXEC_H
#define LIB_ELFLOADER_CTXEXEC_H

#include <bedrock/bedrock.h>

uint32_t exec_init(seL4_CPtr cspace, uint32_t ipc_address, uint8_t priority, uint32_t entry_vector);

bool exec_start(uint32_t cookie_r);

void exec_stop(uint32_t cookie_r);

void exec_destroy(uint32_t cookie_r);

#endif //LIB_ELFLOADER_CTXEXEC_H
