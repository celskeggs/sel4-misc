#include "untyped.h"
#include "../basic.h"
#include "cslot_ao.h"
#include "mem_fx.h"

// tracks two sizes:
// 4 MIB (size of large pages)
// 4 KIB (size of small pages, size of various miscellaneous structures)

struct s_4m {
    seL4_Untyped ut;
    seL4_CPtr aux; // no defined use here. TODO optimize memory use
    struct s_4m *next;
};

struct s_4k {
    seL4_Untyped ut;
    seL4_CPtr aux; // no defined use here. TODO optimize memory use
    struct s_4k *next;
};

static seL4_Error untyped_split(seL4_Untyped ut, int offset, int size_bits, seL4_CPtr slot, int num_objects) {
    assert(ut != seL4_CapNull);
    assert(slot != seL4_CapNull);
    // this implementation is specific to creating untypeds because of weirdness around size_bits handling in the kernel
    while (num_objects > CONFIG_RETYPE_FAN_OUT_LIMIT) {
        seL4_Error err = cslot_retype(ut, seL4_UntypedObject, offset, size_bits, slot,
                                             CONFIG_RETYPE_FAN_OUT_LIMIT);
        if (err != seL4_NoError) {
            return err; // TODO: better cleanup
        }
        num_objects -= CONFIG_RETYPE_FAN_OUT_LIMIT;
        slot += CONFIG_RETYPE_FAN_OUT_LIMIT;
        offset += (1U << size_bits) * CONFIG_RETYPE_FAN_OUT_LIMIT;
    }
    return cslot_retype(ut, seL4_UntypedObject, offset, size_bits, slot, num_objects);
}

#define CACHE_4K_COUNT 4 // todo: decide on this empirically
static struct s_4m *avail_4ms = NULL;
static struct s_4k *avail_4ks = NULL;
static uintptr_t avail_4ks_count = 0; // reflects the length of avail_4ks

// =================== 4M CHUNKS ===================

static seL4_Error untyped2_add_memory_4m(seL4_Untyped ut, seL4_CPtr aux) {
    struct s_4m *node = mem_fx_alloc(sizeof(struct s_4m));
    if (node == NULL) {
        return seL4_NotEnoughMemory;
    }
    node->ut = ut;
    node->aux = aux;
    node->next = avail_4ms;
    avail_4ms = node;
    return seL4_NoError;
}

untyped_4m_ref untyped_allocate_4m(void) {
    if (avail_4ms == NULL) {
        return NULL;
    } else {
        struct s_4m *found = avail_4ms;
        avail_4ms = found->next;
        found->next = NULL;
        return found;
    }
}

seL4_Untyped untyped_ptr_4m(untyped_4m_ref mem) {
    assert(mem != NULL);
    return ((struct s_4m *) mem)->ut;
}

seL4_CPtr untyped_auxptr_4m(untyped_4m_ref mem) {
    assert(mem != NULL);
    return ((struct s_4m *) mem)->aux;
}

void untyped_free_4m(untyped_4m_ref mem) {
    struct s_4m *n = (struct s_4m *) mem;
    assert(n != NULL && n->next == NULL);
    n->next = avail_4ms;
    avail_4ms = n;
    assert(cslot_delete(n->aux) == seL4_NoError);
    assert(cslot_revoke(n->ut) == seL4_NoError);
}

// =================== 4K CHUNKS ===================

static seL4_Error untyped2_add_memory_4k(seL4_Untyped ut, seL4_CPtr aux) {
    struct s_4k *node = mem_fx_alloc(sizeof(struct s_4k));
    if (node == NULL) {
        return seL4_NotEnoughMemory;
    }
    node->ut = ut;
    node->aux = aux;
    node->next = avail_4ks;
    avail_4ks = node;
    avail_4ks_count++;
    return seL4_NoError;
}

static bool refill_running = false;

untyped_4k_ref untyped_allocate_4k(void) {
    if (avail_4ks_count <= CACHE_4K_COUNT && !refill_running && !mem_fx_is_allocating()) {
        assert((avail_4ks_count == 0) == (avail_4ks == NULL));
        // we're running out of memory - refill from 4M pool, assuming that this isn't a recursion while we do that
        refill_running = true;

        untyped_4m_ref larger = untyped_allocate_4m(); // TODO: have some way to rejoin large blocks? maybe?
        seL4_Untyped larger_ut = untyped_ptr_4m(larger);

        uint32_t caps_needed = 1U << (BITS_4MIB - BITS_4KIB);
        seL4_CPtr slots = cslot_ao_alloc(caps_needed * 2); // * 2 so that we have aux slots
        seL4_Error err = untyped_split(larger_ut, 0, BITS_4KIB, slots, caps_needed);
        if (err != seL4_NoError) {
            DEBUG("failed during refill. allocating remnants anyway.");
        } else {
            for (uint32_t i = 0; i < caps_needed; i++) {
                // NOTE: might recurse back to this allocator!
                err = untyped2_add_memory_4k(slots + i, slots + caps_needed + i);
                if (err != seL4_NoError) {
                    // note: we aren't cleaning up properly here... TODO do that.
                    DEBUG("failed during refill. allocating remnants anyway.");
                    break;
                }
            }
        }
        refill_running = false;
        // go and allocate normally now
    }
    if (avail_4ks == NULL) {
        assert(avail_4ks_count == 0);
        return NULL;
    } else {
        assert(avail_4ks_count > 0);
        struct s_4k *found = avail_4ks;
        avail_4ks = found->next;
        found->next = NULL;
        return found;
    }
}

seL4_Untyped untyped_ptr_4k(untyped_4k_ref mem) {
    assert(mem != NULL);
    return ((struct s_4k *) mem)->ut;
}

seL4_CPtr untyped_auxptr_4k(untyped_4k_ref mem) {
    assert(mem != NULL);
    return ((struct s_4k *) mem)->aux;
}

void untyped_free_4k(untyped_4k_ref mem) {
    struct s_4k *n = (struct s_4k *) mem;
    assert(n != NULL && n->next == NULL);
    n->next = avail_4ks;
    avail_4ks = n;
    assert(cslot_delete(n->aux) == seL4_NoError);
    assert(cslot_revoke(n->ut) == seL4_NoError);
}

// =================== MEMORY ADDITION ===================

// if this fails, data structures may be corrupted. see below for reasons.
seL4_Error untyped_add_memory(seL4_Untyped ut, int size_bits) {
    if (size_bits > BITS_4MIB) {
        uint32_t caps_needed = 1U << (size_bits - BITS_4MIB);
        seL4_CPtr slots = cslot_ao_alloc(caps_needed * 2); // * 2 so that we have aux slots
        seL4_Error err = untyped_split(ut, 0, BITS_4MIB, slots, caps_needed);
        if (err != seL4_NoError) {
            return err;
        }
        for (uint32_t i = 0; i < caps_needed; i++) {
            err = untyped2_add_memory_4m(slots + i, slots + caps_needed + i);
            if (err != seL4_NoError) {
                // note: we aren't cleaning up properly here... TODO do that.
                // since this is part of the initialization process, it should be ""okay"" - we'll just crash the system
                return err;
            }
        }
        return seL4_NoError;
    } else if (size_bits == BITS_4MIB) {
        return untyped2_add_memory_4m(ut, cslot_ao_alloc(1));
    } else if (size_bits > BITS_4KIB) {
        uint32_t caps_needed = 1U << (size_bits - BITS_4KIB);
        seL4_CPtr slots = cslot_ao_alloc(caps_needed * 2); // * 2 so that we have aux slots
        seL4_Error err = untyped_split(ut, 0, BITS_4KIB, slots, caps_needed);
        if (err != seL4_NoError) {
            return err;
        }
        for (uint32_t i = 0; i < caps_needed; i++) {
            err = untyped2_add_memory_4k(slots + i, slots + caps_needed + i);
            if (err != seL4_NoError) {
                // note: we aren't cleaning up properly here... TODO do that.
                // since this is part of the initialization process, it should be ""okay"" - we'll just crash the system
                return err;
            }
        }
        return seL4_NoError;
    } else if (size_bits == BITS_4KIB) {
        return untyped2_add_memory_4k(ut, cslot_ao_alloc(1));
    } else {
        assert(size_bits > 0 && size_bits < BITS_4KIB);
        // note: we're wasting these, but it's not very much memory in the big scheme of things
        return seL4_NoError;
    }
}

seL4_Error untyped_add_boot_memory(seL4_BootInfo *info) {
    seL4_Word len = info->untyped.end - info->untyped.start;
    for (seL4_Word i = 0; i < len; i++) {
        seL4_Error err = untyped_add_memory(info->untyped.start + i, info->untypedSizeBitsList[i]);
        if (err != seL4_NoError) {
            return err;
        }
    }
    return seL4_NoError;
}
