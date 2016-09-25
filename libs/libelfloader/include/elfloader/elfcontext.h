#ifndef LIB_ELFLOADER_ELFCONTEXT_H
#define LIB_ELFLOADER_ELFCONTEXT_H

// TCB is not provided to subprocess.

enum elfcontext_cspace {
    ecap_Null = 0,
    ecap_PD,
    ecap_CNode,
    ecap_IPC,
    ecap_IOEP,
    ecap_StartFree,
};

#endif //LIB_ELFLOADER_ELFCONTEXT_H
