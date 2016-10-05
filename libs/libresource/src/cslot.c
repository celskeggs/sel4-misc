#include <resource/cslot.h>
#include <resource/mem_fx.h>

#define BITMAP_ELEMENTS (PAGE_SIZE / 16)
#define BITMAP_BYTES (BITMAP_ELEMENTS / 8)
#define BITMAP_AVAIL(range, i) ((range->avail_bitmap[(i) >> 3] & BIT((i) & 7)) != 0)
#define BITMAP_SET_USED(range, i) (range->avail_bitmap[(i) >> 3] &= ~BIT((i) & 7))
#define BITMAP_SET_AVAIL(range, i) (range->avail_bitmap[(i) >> 3] |= BIT((i) & 7))
seL4_CompileTimeAssert(BITMAP_BYTES == 32);

struct cslot_range {
    seL4_CPtr low;
    seL4_CPtr avail; // less than or equal to the lowest cptr free according to bitmap - optimization
    uint8_t avail_bitmap[BITMAP_BYTES]; // bits are set for available and unset for in use
    struct cslot_range *next;
};

// high is after the allocatable range
static seL4_CNode c_root_cnode;
static struct cslot_range *root_range = NULL;

static bool cslot_insert_range(seL4_CPtr low, seL4_CPtr high) {
    assert(low < high);
    assert(low != seL4_CapNull);
    assert(high != seL4_CapNull);
    assert(high - low <= BITMAP_ELEMENTS);
    struct cslot_range *new_range = mem_fx_alloc(sizeof(struct cslot_range));
    if (new_range == NULL) {
        return false;
    }
    new_range->next = root_range;
    new_range->low = low;
    new_range->avail = low;
    memset(new_range->avail_bitmap, 0xFF, BITMAP_BYTES); // set everything to 'available'
    for (uint32_t i = high - low; i < BITMAP_ELEMENTS; i++) { // TODO: optimize?
        BITMAP_SET_USED(new_range, i); // for bits not included in range
    }
    root_range = new_range;
    return true;
}

static bool cslot_insert_long_range(seL4_CPtr low, seL4_CPtr high) {
    assert(low < high);
    assert(low != seL4_CapNull);
    assert(high != seL4_CapNull);
    while (high - low > BITMAP_ELEMENTS) {
        if (!cslot_insert_range(low, low + BITMAP_ELEMENTS)) {
            return false; // TODO: don't corrupt data in this case?
        }
        low += BITMAP_ELEMENTS;
    }
    return cslot_insert_range(low, high); // TODO: don't corrupt data in this case?
}

bool cslot_setup(seL4_CNode root_cnode, seL4_CPtr low, seL4_CPtr high) {
    assert(root_cnode != seL4_CapNull);
    c_root_cnode = root_cnode;
    return cslot_insert_long_range(low, high);
}

void cslot_free(seL4_CPtr ptr) {
    // TODO: empty the slot?
    assert(ptr != seL4_CapNull);
    for (struct cslot_range *range = root_range; range != NULL; range = range->next) {
        // TODO: possible refuse to accept slots that are outside the originally-allocated area, for terminal cslot ranges?
        if (ptr >= range->low && ptr < range->low + BITMAP_ELEMENTS) {
            assert(!BITMAP_AVAIL(range, ptr - range->low)); // TODO: is crashing the correct behavior on double-free?
            BITMAP_SET_AVAIL(range, ptr - range->low);
            if (ptr < range->avail) {
                range->avail = ptr;
            }
            return;
        }
    }
    fail("attempt to cslot_free cslot outside of allocated bounds");
}

static bool expand_cslot_pool(void) {
    // TODO: should patch in another range dynamically
    ERRX_RAISE_GENERIC(GERR_MEMORY_POOL_EXHAUSTED);
    return false;
}

// here, returning seL4_CapNull is not an error condition, but simply a notification that this range is full.
static seL4_CPtr alloc_from_range(struct cslot_range *range) {
    while (range->avail < range->low + BITMAP_ELEMENTS) {
        if (!BITMAP_AVAIL(range, range->avail - range->low)) {
            range->avail++;
            continue; // try again
        }
        // found an empty slot!
        BITMAP_SET_USED(range, range->avail - range->low);
        return range->avail++;
    }
    return seL4_CapNull;
}

static uint32_t range_count(struct cslot_range *range, uint32_t base) {
    uint32_t count = 0;
    while (base + count < BITMAP_ELEMENTS && BITMAP_AVAIL(range, base + count)) {
        count++;
    }
    return count;
}

static void range_alloc(struct cslot_range *range, uint32_t base, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        assert(BITMAP_AVAIL(range, base + i));
        BITMAP_SET_USED(range, base + i);
    }
}

// here, returning seL4_CapNull is not an error condition, but simply a notification that this range is full.
static seL4_CPtr cslot_alloc_slab_from_range(struct cslot_range *range, uint32_t count) {
    assert(count > 1 && count <= BITMAP_ELEMENTS);
    while (true) {
        if (range->low + BITMAP_ELEMENTS - range->avail < count) {
            return seL4_CapNull; // no way it's going to be enough
        }
        if (BITMAP_AVAIL(range, range->avail - range->low)) {
            break;
        }
        range->avail++;
    }
    uint32_t current_ptr = range->avail;
    while (range->low + BITMAP_ELEMENTS - current_ptr >= count) {
        uint32_t avail_at_current = range_count(range, current_ptr - range->low);
        if (avail_at_current == 0) {
            current_ptr++;
        } else if (avail_at_current >= count) {
            // FOUND IT
            range_alloc(range, current_ptr - range->low, count);
            return current_ptr;
        } else {
            current_ptr += avail_at_current + 1;
        }
    }
    return seL4_CapNull;
}

seL4_CPtr cslot_alloc(void) {
    for (struct cslot_range *range = root_range; range != NULL; range = range->next) {
        seL4_CPtr cptr = alloc_from_range(range);
        if (cptr != seL4_CapNull) {
            return cptr;
        }
    }
    if (!expand_cslot_pool()) {
        ERRX_TRACEPOINT;
        return seL4_CapNull;
    } else {
        seL4_CPtr cptr = alloc_from_range(root_range);
        assert(cptr != seL4_CapNull); // at this point, this really should succeed
        return cptr;
    }
}

seL4_CPtr cslot_alloc_slab(uint32_t count) {
    if (count == 0) {
        ERRX_RAISE_GENERIC(GERR_OUT_OF_RANGE);
        return seL4_CapNull;
    } else if (count == 1) {
        return cslot_alloc();
    } else if (count > BITMAP_ELEMENTS) {
        ERRX_RAISE_GENERIC(GERR_REQUEST_TOO_LARGE);
        return seL4_CapNull;
    }
    for (struct cslot_range *range = root_range; range != NULL; range = range->next) {
        seL4_CPtr cptr = cslot_alloc_slab_from_range(range, count);
        if (cptr != seL4_CapNull) {
            return cptr;
        }
    }
    if (!expand_cslot_pool()) {
        ERRX_TRACEPOINT;
        return seL4_CapNull;
    } else {
        seL4_CPtr cptr = cslot_alloc_slab_from_range(root_range, count);
        assert(cptr != seL4_CapNull); // at this point, this really should succeed
        return cptr;
    }
}

bool cslot_delete(seL4_CPtr ptr) {
    if (ptr == seL4_CapNull) {
        ERRX_RAISE_GENERIC(GERR_NULL_VALUE);
        return false;
    }
    int err = seL4_CNode_Delete(c_root_cnode, ptr, 32);
    if (err == seL4_NoError) {
        return true;
    } else {
        ERRX_RAISE_SEL4(err);
        return false;
    }
}

bool cslot_copy(seL4_CPtr from, seL4_CPtr to) {
    if (from == seL4_CapNull || to == seL4_CapNull) {
        ERRX_RAISE_GENERIC(GERR_NULL_VALUE);
        return false;
    }
    int err = seL4_CNode_Copy(c_root_cnode, to, 32, c_root_cnode, from, 32, seL4_AllRights);
    if (err == seL4_NoError) {
        return true;
    } else {
        ERRX_RAISE_SEL4(err);
        return false;
    }
}

bool cslot_copy_out(seL4_CPtr from, seL4_CNode to_node, seL4_Word to, uint8_t to_depth) {
    if (from == seL4_CapNull || to_node == seL4_CapNull) {
        ERRX_RAISE_GENERIC(GERR_NULL_VALUE);
        return false;
    }
    // note: if you get "Target slot invalid" (via debug message) and "FailedLookup" through interface, check bit depth.
    int err = seL4_CNode_Copy(to_node, to, to_depth, c_root_cnode, from, 32, seL4_AllRights);
    if (err == seL4_NoError) {
        return true;
    } else {
        ERRX_RAISE_SEL4(err);
        return false;
    }
}

bool cslot_mint(seL4_CPtr from, seL4_CPtr to, uint32_t badge) {
    if (from == seL4_CapNull || to == seL4_CapNull) {
        ERRX_RAISE_GENERIC(GERR_NULL_VALUE);
        return false;
    }
    int err = seL4_CNode_Mint(c_root_cnode, to, 32, c_root_cnode, from, 32, seL4_AllRights,
                              seL4_CapData_Badge_new(badge));
    if (err == seL4_NoError) {
        return true;
    } else {
        ERRX_RAISE_SEL4(err);
        return false;
    }
}

bool cslot_mint_out(seL4_CPtr from, seL4_CNode to_node, seL4_Word to, uint8_t to_depth, uint32_t badge) {
    if (from == seL4_CapNull || to_node == seL4_CapNull) {
        ERRX_RAISE_GENERIC(GERR_NULL_VALUE);
        return false;
    }
    int err = seL4_CNode_Mint(to_node, to, to_depth, c_root_cnode, from, 32, seL4_AllRights,
                              seL4_CapData_Badge_new(badge));
    if (err == seL4_NoError) {
        return true;
    } else {
        ERRX_RAISE_SEL4(err);
        return false;
    }
}

bool cslot_revoke(seL4_CPtr ptr) {
    if (ptr == seL4_CapNull) {
        ERRX_RAISE_GENERIC(GERR_NULL_VALUE);
        return false;
    }
    int err = seL4_CNode_Revoke(c_root_cnode, ptr, 32);
    if (err == seL4_NoError) {
        return true;
    } else {
        ERRX_RAISE_SEL4(err);
        return false;
    }
}

bool cslot_retype(seL4_Untyped ut, int type, int offset, int size_bits, seL4_CPtr slot, int num_objects) {
    if (ut == seL4_CapNull || slot == seL4_CapNull) {
        ERRX_RAISE_GENERIC(GERR_NULL_VALUE);
        return false;
    }
    int err = seL4_Untyped_RetypeAtOffset(ut, type, offset, size_bits, c_root_cnode, 0, 0, slot, num_objects);
    if (err == seL4_NoError) {
        return true;
    } else {
        ERRX_RAISE_SEL4(err);
        return false;
    }
}

bool cslot_irqget(seL4_IRQControl ctrl, int irq, seL4_CPtr slot) {
    if (ctrl == seL4_CapNull || slot == seL4_CapNull) {
        ERRX_RAISE_GENERIC(GERR_NULL_VALUE);
        return false;
    }
    int err = seL4_IRQControl_Get(ctrl, irq, c_root_cnode, slot, 32);
    if (err == seL4_NoError) {
        return true;
    } else {
        ERRX_RAISE_SEL4(err);
        return false;
    }
}
