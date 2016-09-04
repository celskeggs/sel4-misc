#ifndef SEL4_MISC_MEM_FX_H
#define SEL4_MISC_MEM_FX_H

#include "../basic.h"

#define MEM_FX_DECL(t,x) t x = mem_fx_alloc(sizeof(t))
#define MEM_FX_ALLOC(x) x = mem_fx_alloc(sizeof(*x))
#define MEM_FX_FREE(x) mem_fx_free(x, sizeof(*x))

seL4_Error mem_fx_init(void);
void *mem_fx_alloc(size_t size);
void mem_fx_free(void *data, size_t size);

#endif //SEL4_MISC_MEM_FX_H
