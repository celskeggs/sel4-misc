#ifndef LIB_ELFLOADER_ELFCONTEXT_H
#define LIB_ELFLOADER_ELFCONTEXT_H

// TCB is not provided to subprocess.
#include <bedrock/kmem.h>

// TODO: get 4 (log 16, the cnode entry size) as a constant
#define ECAP_ROOT_BITS (BITS_4KIB - 4)

enum elfcontext_cspace {
    ecap_Null = 0,
    ecap_PD,
    ecap_CNode,
    ecap_IPC,
    ecap_IOEP,
    ecap_StartFree,
    ecap_EndFree = BIT(ECAP_ROOT_BITS) - 1,
};

#endif //LIB_ELFLOADER_ELFCONTEXT_H
