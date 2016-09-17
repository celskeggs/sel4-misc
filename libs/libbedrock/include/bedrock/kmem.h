#ifndef LIB_BEDROCK_KMEM_H
#define LIB_BEDROCK_KMEM_H

#include <sel4/types.h>

#define PAGE_SIZE (1U << seL4_PageBits)
#define PAGE_TABLE_SIZE (1U << seL4_LargePageBits)
#define KERNEL_BASE_VADDR 0xE0000000ULL

#endif //LIB_BEDROCK_KMEM_H
