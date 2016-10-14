#ifndef APP_INIT_UNTYPED_H
#define APP_INIT_UNTYPED_H

#include <bedrock/bedrock.h>
#include <bedrock/errx.h> // this module uses errx

// if this fails, data structures may be corrupted
bool untyped_add_boot_memory(seL4_BootInfo *info);

typedef void *untyped_4k_ref;

untyped_4k_ref untyped_allocate_4k(void);

seL4_CPtr untyped_ptr_4k(untyped_4k_ref mem);

seL4_CPtr untyped_auxptr_4k(untyped_4k_ref mem);

void untyped_free_4k(untyped_4k_ref mem);

untyped_4k_ref untyped_allocate_retyped(int type);

#endif //APP_INIT_UNTYPED_MACRO_H
