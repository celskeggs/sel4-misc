#ifndef SEL4_MISC_CSLOT_AO_H
#define SEL4_MISC_CSLOT_AO_H

#include <sel4/sel4.h>
#include "../basic.h"

void cslot_ao_setup(seL4_CNode root_cnode, seL4_CPtr low, seL4_CPtr high);

seL4_CPtr cslot_ao_alloc(uint32_t count);

void cslot_ao_dealloc_last(seL4_CPtr ptr);

// these three are necessary because SOMEONE has to know the CSpace layout
seL4_Error cslot_delete(seL4_CPtr ptr);

seL4_Error cslot_revoke(seL4_CPtr ptr);

seL4_Error cslot_retype(seL4_Untyped ut, int type, int offset, int size_bits, seL4_CPtr slot, int num_objects);

seL4_Error cslot_irqget(seL4_IRQControl ctrl, int irq, seL4_CPtr slot);

#endif //SEL4_MISC_CSLOT_AO_H
