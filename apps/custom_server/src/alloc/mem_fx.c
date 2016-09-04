#include "mem_fx.h"
#include "mem_ao.h"
#include "mem_multislab.h"

static bool fx_initialized = false;
static bool fx_initializing = false;
static struct mem_multislab fx_multislab;

seL4_Error mem_fx_init(void) {
    if (fx_initialized) {
        return seL4_NoError;
    }
    if (fx_initializing) {
        return seL4_IllegalOperation;
    }
    fx_initializing = true;
    seL4_Error err = mem_multislab_create(&fx_multislab);
    fx_initializing = false;
    assert(!fx_initialized);
    if (err != seL4_NoError) {
        return err;
    }
    fx_initialized = true;
    return seL4_NoError;
}

void *mem_fx_alloc(size_t size) {
    assert(size > 0 && size <= MAX_ALLOWED_MULTISLAB_ALLOCATION);
    if (fx_initialized) {
        return mem_multislab_malloc(&fx_multislab, size);
    } else {
        return mem_ao_alloc(size);
    }
}

void mem_fx_free(void *data, size_t size) {
    assert(size > 0 && size <= MAX_ALLOWED_MULTISLAB_ALLOCATION);
    if (fx_initialized) {
        mem_multislab_free(&fx_multislab, data, size);
    } else if (mem_ao_is_last(data, size)) {
        mem_ao_dealloc_last(data, size);
    } else {
        DEBUG("LEAKING MEMORY");
    }
}
