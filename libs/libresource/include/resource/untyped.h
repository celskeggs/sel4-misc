#ifndef LIB_RESOURCE_UNTYPED_H
#define LIB_RESOURCE_UNTYPED_H

#include <bedrock/bedrock.h>
#include <bedrock/errx.h> // this module uses errx

// if this fails, data structures may be corrupted
bool untyped_add_boot_memory(seL4_BootInfo *info);

// if this fails, data structures may be corrupted
bool untyped_add_memory(seL4_Untyped ut, int size_bits);

// necessary memory sizes:
// 16 bytes (endpoint, notification)
// 1 kib (TCB, small 64-slot cnodes)
// 4 kib (pages, various IA32 structures, 256-slot cnodes)
// 4 mib (large pages)
// we just provide two of these in this module. see object.h for 16-byte allocation.

typedef void *untyped_4k_ref;

untyped_4k_ref untyped_allocate_4k(void);

seL4_Untyped untyped_ptr_4k(untyped_4k_ref mem);

seL4_CPtr untyped_auxptr_4k(untyped_4k_ref mem);

void untyped_free_4k(untyped_4k_ref mem);

#endif //LIB_RESOURCE_UNTYPED_MACRO_H
