#ifndef SEL4_MISC_CSLOT_AO_H
#define SEL4_MISC_CSLOT_AO_H

#include <sel4/sel4.h>
#include "../basic.h"

void cslot_ao_setup(seL4_CPtr low, seL4_CPtr high);
seL4_CPtr cslot_ao_alloc(uint32_t count);
void cslot_ao_dealloc_last(seL4_CPtr ptr);

#endif //SEL4_MISC_CSLOT_AO_H
