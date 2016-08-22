#include "cslot.h"
#include "../basic.h"

struct cslot_free {
    seL4_CPtr slot;
    struct cslot_free *next;
};

struct cslot_range {
    seL4_CPtr low, high;
    seL4_CPtr next;
    struct cslot_free *free;
    struct cslot_range *next_range;
};

struct cslot_range *ranges = NULL;

void cslot_register_range(seL4_CPtr low, seL4_CPtr high) {
    struct cslot_range *new_range = malloc(sizeof(struct cslot_range));
    new_range->low = new_range->next = low;
    new_range->high = high;
    new_range->next_range = NULL;
    new_range->free = NULL;
    if (ranges == NULL) {
        ranges = new_range;
    } else {
        struct cslot_range *last_range = ranges;
        while (last_range->next_range != NULL) {
            last_range = last_range->next_range;
        }
        last_range->next_range = new_range;
    }
}

seL4_CPtr cslot_range_allocate(struct cslot_range *range) {
    struct cslot_free *free_slot = range->free;
    if (free_slot != NULL) {
        seL4_CPtr slot = free_slot->slot;
        range->free = free_slot->next;
        free(free_slot);
        return slot;
    }
    if (range->next < range->high) {
        return range->next++;
    }
    return seL4_CapNull;
}

bool cslot_range_includes(struct cslot_range *range, seL4_CPtr ptr) {
    return ptr >= range->low && ptr < range->high;
}

void cslot_range_free(struct cslot_range *range, seL4_CPtr ptr) {
    assert(cslot_range_includes(range, ptr));
    if (ptr == range->next - 1) {
        range->next--;
    } else {
        struct cslot_free *slot = malloc(sizeof(struct cslot_free));
        slot->next = range->free;
        slot->slot = ptr;
        range->free = slot;
    }
}

seL4_CPtr cslot_allocate() {
    struct cslot_range *cur = ranges;
    while (cur != NULL) {
        seL4_CPtr ptr = cslot_range_allocate(cur);
        if (ptr != seL4_CapNull) {
            return ptr;
        }
        cur = cur->next_range;
    }
    return seL4_CapNull;
}

void cslot_free(seL4_CPtr ptr) {
    if (ptr == seL4_CapNull) {
        return;
    }
    struct cslot_range *cur = ranges;
    while (cur != NULL) {
        if (cslot_range_includes(cur, ptr)) {
            cslot_range_free(cur, ptr);
            return;
        }
        cur = cur->next_range;
    }
    assert(!"cslot was not allocated from this range!");
}
