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
    ERRX_RAISE_GENERIC(GERR_MEMORY_POOL_EXHAUSTED);
    return seL4_CapNull;
}

bool cslot_delete(seL4_CPtr ptr) {
    int err = seL4_CNode_Delete(c_root_cnode, ptr, 32);
    if (err == seL4_NoError) {
        return true;
    } else {
        ERRX_RAISE_SEL4(err);
        return false;
    }
}

bool cslot_copy(seL4_CPtr from, seL4_CPtr to) {
    int err = seL4_CNode_Copy(c_root_cnode, to, 32, c_root_cnode, from, 32, seL4_AllRights);
    if (err == seL4_NoError) {
        return true;
    } else {
        ERRX_RAISE_SEL4(err);
        return false;
    }
}

bool cslot_revoke(seL4_CPtr ptr) {
    int err = seL4_CNode_Revoke(c_root_cnode, ptr, 32);
    if (err == seL4_NoError) {
        return true;
    } else {
        ERRX_RAISE_SEL4(err);
        return false;
    }
}

bool cslot_retype(seL4_Untyped ut, int type, int offset, int size_bits, seL4_CPtr slot, int num_objects) {
    int err = seL4_Untyped_RetypeAtOffset(ut, type, offset, size_bits, c_root_cnode, 0, 0, slot, num_objects);
    if (err == seL4_NoError) {
        return true;
    } else {
        ERRX_RAISE_SEL4(err);
        return false;
    }
}

bool cslot_irqget(seL4_IRQControl ctrl, int irq, seL4_CPtr slot) {
    int err = seL4_IRQControl_Get(ctrl, irq, c_root_cnode, slot, 32);
    if (err == seL4_NoError) {
        return true;
    } else {
        ERRX_RAISE_SEL4(err);
        return false;
    }
}
