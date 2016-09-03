#include <sel4/sel4.h>
#include "../basic.h"
#include "untyped.h"
#include "mem_ao.h"
#include "cslot_ao.h"

static seL4_Error retype(seL4_Untyped ut, int type, int offset, int size_bits, seL4_CPtr slot, int num_objects) {
    assert(ut != seL4_CapNull);
    assert(slot != seL4_CapNull);
    // root only
    return seL4_Untyped_RetypeAtOffset(ut, type, offset, size_bits, seL4_CapInitThreadCNode, 0, 0, slot, num_objects);
}

seL4_Error untyped_retype_one(seL4_Untyped ut, int type, int offset, int size_bits, seL4_CPtr slot) {
    // root only
    return retype(ut, type, offset, size_bits, slot, 1);
}

static int split_untyped(seL4_Untyped ut, seL4_Untyped outA, seL4_Untyped outB, int size_orig) {
    seL4_Error err = untyped_retype_one(ut, seL4_UntypedObject, 0, size_orig - 1, outA);
    if (err == seL4_NoError) {
        err = untyped_retype_one(ut, seL4_UntypedObject, 1 << (size_orig - 1), size_orig - 1, outB);
        if (err != seL4_NoError) {
            assert(seL4_CNode_Delete(seL4_CapInitThreadCNode, outA, 32) == seL4_NoError);
        }
    }
    return err;
}

static void join_untyped(seL4_Untyped inA, seL4_Untyped inB) {
    // all we need to do is delete the caps
    assert(seL4_CNode_Delete(seL4_CapInitThreadCNode, inA, 32) == seL4_NoError);
    assert(seL4_CNode_Delete(seL4_CapInitThreadCNode, inB, 32) == seL4_NoError);
}

// largest possible block: 2^32
// smallest possible block: 2^10 (a kilobyte)
// although seL4 supports smaller blocks, we aren't going to allocate them in this allocator.

#define EXPON_MIN 10
#define EXPON_MAX 32
#define EXPON_COUNT (EXPON_MAX - EXPON_MIN + 1)

struct buddy_block {
    seL4_Untyped ut; // constant for each allocated struct
    bool allocated;
    // for the tree
    struct buddy_block *parent;
    struct buddy_block *sibling;
    // for the linked list
    struct buddy_block *next;
};

// 0th index: with size bit of one
static struct buddy_block *linked_lists[EXPON_COUNT] = {NULL, NULL, NULL, NULL};
static struct buddy_block *allocated_linked_lists[EXPON_COUNT] = {NULL, NULL, NULL, NULL};
static struct buddy_block *free_blocks = NULL;

static struct buddy_block *alloc_block_struct(void) {
    if (free_blocks == NULL) {
        struct buddy_block *block = (struct buddy_block *) mem_ao_alloc(sizeof(struct buddy_block));
        block->ut = cslot_ao_alloc();
        if (block->ut == seL4_CapNull) {
            mem_ao_dealloc_last(block, sizeof(struct buddy_block));
            return NULL;
        }
        block->allocated = false;
        return block;
    } else {
        struct buddy_block *out = free_blocks;
        free_blocks = free_blocks->next;
        return out;
    }
}

static void free_block_struct(struct buddy_block *block) {
    assert(block != NULL);
    assert(!block->allocated);
    block->parent = NULL;
    block->sibling = NULL;
    block->next = free_blocks;
    free_blocks = block;
}

static void free_untyped(int size_bits, struct buddy_block *block);

static struct buddy_block *allocate_untyped(int size_bits) {
    assert(size_bits >= EXPON_MIN && size_bits <= EXPON_MAX);
    int index = size_bits - EXPON_MIN;
    struct buddy_block *block = linked_lists[index];
    if (block != NULL) {
        assert(!block->allocated);
        block->allocated = true;
        linked_lists[index] = block->next;
        block->next = allocated_linked_lists[index];
        allocated_linked_lists[index] = block;
        return block;
    } else if (size_bits + 1 > EXPON_MAX) {
        DEBUG("out of memory");
        return NULL; // out of memory
    } else {
        struct buddy_block *childA = alloc_block_struct();
        if (childA == NULL) {
            DEBUG("no struct A");
            return NULL;
        }
        struct buddy_block *childB = alloc_block_struct();
        if (childB == NULL) {
            DEBUG("no struct B");
            free_block_struct(childA);
            return NULL;
        }
        struct buddy_block *parent = allocate_untyped(size_bits + 1);
        if (parent == NULL) {
            DEBUG("could not alloc larger");
            free_block_struct(childA);
            free_block_struct(childB);
            return NULL;
        }
        assert(parent->allocated);
        if (split_untyped(parent->ut, childA->ut, childB->ut, size_bits + 1) != seL4_NoError) {
            DEBUG("could not split untyped");
            free_block_struct(childA);
            free_block_struct(childB);
            free_untyped(size_bits + 1, parent);
            return NULL;
        }
        childA->allocated = true;
        childB->allocated = false;
        childA->parent = childB->parent = parent;
        childA->sibling = childB;
        childB->sibling = childA;
        childA->next = allocated_linked_lists[index];
        allocated_linked_lists[index] = childA;
        assert(linked_lists[index] == NULL);
        childB->next = NULL; // already known to be empty
        linked_lists[index] = childB;
        return childA;
    }
}

static void remove_from_allocated_list(int index, struct buddy_block *block) {
    assert(block->allocated);
    if (block == allocated_linked_lists[index]) {
        allocated_linked_lists[index] = block->next;
    } else {
        struct buddy_block *iter = allocated_linked_lists[index];
        assert(iter != NULL);
        while (iter->next != block) {
            assert(iter->next != NULL); // fail if the block doesn't actually exist
            iter = iter->next;
        }
        iter->next = block->next;
    }
    block->next = NULL;
}

static void remove_from_linked_list(int index, struct buddy_block *block) {
    assert(!block->allocated);
    if (block == linked_lists[index]) {
        linked_lists[index] = block->next;
    } else {
        struct buddy_block *iter = linked_lists[index];
        assert(iter != NULL);
        while (iter->next != block) {
            assert(iter->next != NULL); // fail if the block doesn't actually exist
            iter = iter->next;
        }
        iter->next = block->next;
    }
    block->next = NULL;
}

static void free_untyped(int size_bits, struct buddy_block *block) {
    assert(size_bits >= EXPON_MIN && size_bits <= EXPON_MAX);
    assert(block != NULL);
    int index = size_bits - EXPON_MIN;
    remove_from_allocated_list(index, block);
    block->allocated = false;

    if (size_bits == EXPON_MAX) {
        assert(block->sibling == NULL);
        return;
    }
    // now we try to join it with its sibling
    assert(block->sibling != NULL && block->sibling->sibling == block);
    if (block->sibling->allocated) {
        // cannot join, so just add us to the list
        block->next = linked_lists[index];
        linked_lists[index] = block;
    } else {
        assert(block->sibling->parent == block->parent);
        // ooh! we're a pair - remove sibling from linked list
        remove_from_linked_list(index, block->sibling);
        // time to join!
        join_untyped(block->ut, block->sibling->ut); // TODO: SHOULD WE REVOKE SOMETHING HERE?
        struct buddy_block *parent = block->parent;
        free_block_struct(block->sibling);
        free_block_struct(block);
        free_untyped(size_bits + 1, parent);
    }
}

seL4_Error untyped_add_memory(seL4_Untyped ut, int size_bits) {
    struct buddy_block *block = alloc_block_struct();
    if (block == NULL) {
        return seL4_NotEnoughMemory;
    }
    assert(size_bits >= EXPON_MIN && size_bits <= EXPON_MAX);
    int index = size_bits - EXPON_MIN;
    block->ut = ut;
    block->allocated = false;
    block->next = linked_lists[index];
    linked_lists[index] = block;
    block->parent = NULL;
    block->sibling = NULL;
    return seL4_NoError;
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

untyped_ref untyped_alloc(uint8_t size_bits) {
    assert(size_bits >= EXPON_MIN && size_bits <= EXPON_MAX);
    return allocate_untyped(size_bits);
}

void untyped_dealloc(uint8_t size_bits, untyped_ref ref) {
    assert(size_bits >= EXPON_MIN && size_bits <= EXPON_MAX);
    free_untyped(size_bits, ref);
}

seL4_Untyped untyped_ptr(untyped_ref ref) {
    struct buddy_block *bb = ref;
    assert(bb->allocated);
    return bb->ut;
}

seL4_Error untyped_retype_to(untyped_ref ref, int type, int offset, int size_bits, seL4_CPtr ptr) {
    return untyped_retype_one(untyped_ptr(ref), type, offset, size_bits, ptr);
}

seL4_CPtr untyped_retype(untyped_ref ref, int type, int offset, int size_bits) {
    seL4_CPtr slot = cslot_ao_alloc();
    if (slot == seL4_CapNull) {
        return seL4_CapNull;
    }
    if (untyped_retype_one(untyped_ptr(ref), type, offset, size_bits, slot) != seL4_NoError) {
        // todo: make sure the slot is empty before deallocating it
        cslot_ao_dealloc_last(slot);
        return seL4_CapNull;
    }
    return slot;
}

void untyped_detype(seL4_CPtr ptr) {
    assert(seL4_CNode_Delete(seL4_CapInitThreadCNode, ptr, 32) == seL4_NoError);
}
