#ifndef SEL4_MISC_UNTYPED_H
#define SEL4_MISC_UNTYPED_H

#include <bedrock/bedrock.h>

// if this fails, data structures may be corrupted
seL4_Error untyped_add_boot_memory(seL4_BootInfo *info);

// if this fails, data structures may be corrupted
seL4_Error untyped_add_memory(seL4_Untyped ut, int size_bits);

// necessary memory sizes:
// 16 bytes (endpoint, notification)
// 1 kib (TCB, small 64-slot cnodes)
// 4 kib (pages, various IA32 structures, 256-slot cnodes)
// 4 mib (large pages)

#define BITS_4KIB 12
#define BITS_4MIB 22

seL4_CompileTimeAssert(seL4_PageBits == BITS_4KIB); // 4 kib
seL4_CompileTimeAssert(seL4_LargePageBits == BITS_4MIB); // 4 mib

typedef void *untyped_4m_ref;
typedef void *untyped_4k_ref;


// just a helper function
seL4_Error untyped_root_retype(seL4_Untyped ut, int type, int offset, int size_bits, seL4_CPtr slot, int num_objects);


untyped_4m_ref untyped_allocate_4m(void);

seL4_Untyped untyped_ptr_4m(untyped_4m_ref mem);

seL4_CPtr untyped_auxptr_4m(untyped_4m_ref mem);

void untyped_free_4m(untyped_4m_ref mem);


untyped_4k_ref untyped_allocate_4k(void);

seL4_Untyped untyped_ptr_4k(untyped_4k_ref mem);

seL4_CPtr untyped_auxptr_4k(untyped_4k_ref mem);

void untyped_free_4k(untyped_4k_ref mem);

/* for micro
seL4_Untyped untyped2_allocate_1k();
void untyped2_free_1k(seL4_Untyped untyped);
seL4_Untyped untyped2_allocate_16();
void untyped2_free_16(seL4_Untyped untyped);
*/

#endif //SEL4_MISC_UNTYPED_MACRO_H
