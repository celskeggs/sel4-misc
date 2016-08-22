#ifndef SEL4_MISC_UNTYPED_H
#define SEL4_MISC_UNTYPED_H

#include <sel4/sel4.h>

typedef void *untyped_ref;
#define UNTYPED_NONE ((untyped_ref) NULL)

seL4_Error untyped_retype_one(seL4_Untyped ut, int type, int offset, int size_bits, seL4_CPtr slot);

seL4_Error untyped_add_boot_memory(seL4_BootInfo *info);
seL4_Error untyped_add_memory(seL4_Untyped ut, int size_bits);

untyped_ref untyped_alloc(uint8_t size_bits);
void untyped_dealloc(uint8_t size_bits, untyped_ref ref);
seL4_Untyped untyped_ptr(untyped_ref ref);
// allocated with cslot_ao_alloc
seL4_CPtr untyped_retype(untyped_ref ref, int type, int offset, int size_bits);
seL4_Error untyped_retype_to(untyped_ref ref, int type, int offset, int size_bits, seL4_CPtr ptr);

#endif //SEL4_MISC_UNTYPED_H
