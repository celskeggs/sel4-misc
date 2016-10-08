#ifndef LIB_RESOURCE_CSLOT_AO_H
#define LIB_RESOURCE_CSLOT_AO_H

#include <bedrock/bedrock.h>
#include <bedrock/errx.h> // this module uses errx

bool cslot_setup(seL4_CNode root_cnode, seL4_CPtr low, seL4_CPtr high);

seL4_CPtr cslot_alloc(void); // TODO: make sure that all users check error conditions

seL4_CPtr cslot_alloc_slab(uint32_t count);

void cslot_free(seL4_CPtr ptr);

// these three are necessary because SOMEONE has to know the CSpace layout
// TODO: look at all of the uses of these to make sure they're used properly in the context of errx.
bool cslot_delete(seL4_CPtr ptr);

bool cslot_copy(seL4_CPtr from, seL4_CPtr to);

bool cslot_copy_out(seL4_CPtr from, seL4_CNode to_node, seL4_Word to, uint8_t to_depth);

bool cslot_mutate(seL4_CPtr from, seL4_CPtr to, seL4_CapData_t cdata);

bool cslot_mint(seL4_CPtr from, seL4_CPtr to, uint32_t badge);

bool cslot_mint_out(seL4_CPtr from, seL4_CNode to_node, seL4_Word to, uint8_t to_depth, uint32_t badge);

bool cslot_revoke(seL4_CPtr ptr);

bool cslot_retype(seL4_Untyped ut, int type, int offset, int size_bits, seL4_CPtr slot, int num_objects);

bool cslot_irqget(seL4_IRQControl ctrl, int irq, seL4_CPtr slot);

bool cslot_set_receive_path(seL4_CPtr ptr);

#endif //LIB_RESOURCE_CSLOT_AO_H
