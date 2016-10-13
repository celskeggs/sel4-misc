#ifndef LIB_BEDROCK_KMEM_H
#define LIB_BEDROCK_KMEM_H

#include <sel4/types.h>

#define BITS_4KIB 12
#define BITS_4MIB 22
#define PAGE_SIZE (1U << seL4_PageBits)
#define PAGE_TABLE_SIZE (1U << seL4_LargePageBits)
#define KERNEL_BASE_VADDR 0xE0000000ULL
#define PAGE_TABLE_COUNT (KERNEL_BASE_VADDR / PAGE_TABLE_SIZE)
#define PAGE_COUNT_PER_TABLE (PAGE_TABLE_SIZE / PAGE_SIZE)
seL4_CompileTimeAssert(seL4_PageBits == BITS_4KIB); // 4 kib
seL4_CompileTimeAssert(seL4_LargePageBits == BITS_4MIB); // 4 mib
seL4_CompileTimeAssert(PAGE_SIZE == 4096);
seL4_CompileTimeAssert(PAGE_TABLE_SIZE == 4194304);
seL4_CompileTimeAssert(PAGE_TABLE_COUNT == 896);
seL4_CompileTimeAssert(PAGE_COUNT_PER_TABLE == 1024);

#endif //LIB_BEDROCK_KMEM_H
