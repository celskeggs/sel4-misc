#include <resource/cslot_ao.h>

// high is after the allocatable range
static seL4_CNode c_root_cnode;
static seL4_CPtr c_low = seL4_CapNull, c_high = seL4_CapNull, c_next = seL4_CapNull;

void cslot_ao_setup(seL4_CNode root_cnode, seL4_CPtr low, seL4_CPtr high) {
    assert(root_cnode != seL4_CapNull);
    assert(c_low == seL4_CapNull && c_high == seL4_CapNull && c_next == seL4_CapNull);
    assert(low < high);
    assert(low != seL4_CapNull && high != seL4_CapNull);
    c_root_cnode = root_cnode;
    c_low = low;
    c_high = high;
    c_next = low;
}

void cslot_ao_dealloc_last(seL4_CPtr ptr) {
    // TODO: empty the slot?
    assert(ptr != seL4_CapNull);
    assert(ptr >= c_low);
    assert(ptr == c_next - 1);
    c_next--;
}

seL4_CPtr cslot_ao_alloc(uint32_t count) {
    assert(c_next != seL4_CapNull);
    if (c_next + count > c_next && c_next + count <= c_high) {
        seL4_CPtr out = c_next;
        c_next += count;
        return out;
    }
    return seL4_CapNull;
}

seL4_Error cslot_delete(seL4_CPtr ptr) {
    return (seL4_Error) seL4_CNode_Delete(c_root_cnode, ptr, 32);
}

seL4_Error cslot_copy(seL4_CPtr from, seL4_CPtr to) {
    return (seL4_Error) seL4_CNode_Copy(c_root_cnode, to, 32, c_root_cnode, from, 32, seL4_AllRights);
}

seL4_Error cslot_revoke(seL4_CPtr ptr) {
    return (seL4_Error) seL4_CNode_Revoke(c_root_cnode, ptr, 32);
}

seL4_Error cslot_retype(seL4_Untyped ut, int type, int offset, int size_bits, seL4_CPtr slot, int num_objects) {
    return (seL4_Error) seL4_Untyped_RetypeAtOffset(ut, type, offset, size_bits, c_root_cnode, 0, 0, slot,
                                                    num_objects);
}

seL4_Error cslot_irqget(seL4_IRQControl ctrl, int irq, seL4_CPtr slot) {
    return (seL4_Error) seL4_IRQControl_Get(ctrl, irq, c_root_cnode, slot, 32);
}
