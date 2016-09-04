#ifndef SEL4_MISC_MEM_AO_H
#define SEL4_MISC_MEM_AO_H

#include "../basic.h"

void *mem_ao_alloc(size_t len);
bool mem_ao_is_last(void *ptr, size_t len);
void mem_ao_dealloc_last(void *ptr, size_t len);

#endif //SEL4_MISC_MEM_AO_H
