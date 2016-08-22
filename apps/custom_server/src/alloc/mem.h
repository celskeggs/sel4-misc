#include "../basic.h"

#ifndef SEL4_MISC_MEM_H
#define SEL4_MISC_MEM_H

void *mem_alloc(size_t len);
void mem_dealloc(void *ptr, size_t len);

#endif //SEL4_MISC_MEM_H
