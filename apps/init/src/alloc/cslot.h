#ifndef SEL4_MISC_CSLOT_H
#define SEL4_MISC_CSLOT_H

#include <sel4/sel4.h>

void cslot_register_range(seL4_CPtr low, seL4_CPtr high);
seL4_CPtr cslot_allocate();
void cslot_free(seL4_CPtr ptr);

#endif //SEL4_MISC_CSLOT_H
