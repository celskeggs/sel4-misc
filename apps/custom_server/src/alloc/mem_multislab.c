#include "mem_multislab.h"

seL4_Error mem_multislab_create(struct mem_multislab *multislab) {
    seL4_Error err = mem_arena_ao_create(&multislab->arena);
    if (err != seL4_NoError) {
        return err;
    }
    for (uint16_t i = 0; i < MULTISLAB_SUBSLAB_COUNT; i++) {
        err = mem_slab_create(&multislab->subslabs[i], (i + 1) * MULTISLAB_ALLOC_MULTIPLE);
        if (err != seL4_NoError) {
            return err;
        }
    }
    return seL4_NoError;
}

void *mem_multislab_malloc(struct mem_multislab *multislab, size_t size) {
    uint32_t index = (size - 1) / MULTISLAB_ALLOC_MULTIPLE; // 1,2,3,4 bytes -> 0th. 5,6,7,8 bytes -> 1st. etc.
    assert(index < MULTISLAB_SUBSLAB_COUNT);
    return mem_slab_malloc(&multislab->subslabs[index], &multislab->arena);
}

void mem_multislab_free(struct mem_multislab *multislab, void *ptr, size_t size) {
    uint32_t index = (size - 1) / MULTISLAB_ALLOC_MULTIPLE; // 1,2,3,4 bytes -> 0th. 5,6,7,8 bytes -> 1st. etc.
    assert(index < MULTISLAB_SUBSLAB_COUNT);
    mem_slab_free(&multislab->subslabs[index], ptr);
}

void mem_multislab_destroy(struct mem_multislab *multislab) {
    mem_arena_ao_destroy(&multislab->arena);
}