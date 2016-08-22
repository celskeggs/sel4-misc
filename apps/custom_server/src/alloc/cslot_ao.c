#include "cslot_ao.h"

// high is after the allocatable range
static seL4_CPtr c_low = seL4_CapNull, c_high = seL4_CapNull, c_next = seL4_CapNull;

void cslot_ao_setup(seL4_CPtr low, seL4_CPtr high) {
    assert(c_low == seL4_CapNull && c_high == seL4_CapNull && c_next == seL4_CapNull);
    assert(low < high);
    assert(low != seL4_CapNull && high != seL4_CapNull);
    c_low = low;
    c_high = high;
    c_next = low;
}

seL4_CPtr cslot_ao_alloc() {
    assert(c_next != seL4_CapNull);
    if (c_next < c_high) {
        return c_next++;
    }
    return seL4_CapNull;
}

void cslot_ao_dealloc_last(seL4_CPtr ptr) {
    assert(ptr != seL4_CapNull);
    assert(ptr >= c_low);
    assert(ptr == c_next - 1);
    c_next--;
}

seL4_CPtr cslot_ao_alloc_slab(uint32_t count) {
    assert(c_next != seL4_CapNull);
    if (c_next + count > c_next && c_next + count <= c_high) {
        seL4_CPtr out = c_next;
        c_next += count;
        return out;
    }
    return seL4_CapNull;
}
