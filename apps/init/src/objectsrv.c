#include <resource/object.h>
#include "small.h"
#include "untyped.h"
#include <resource/mem_fx.h>

struct small_tok {
    seL4_CPtr ptr;
};

object_token object_alloc(uint32_t object_type) {
    switch (object_type) {
        case seL4_EndpointObject:
        case seL4_NotificationObject: {
            struct small_tok *st = mem_fx_alloc(sizeof(struct small_tok));
            if (st == NULL) {
                return NULL;
            }
            assert(((uintptr_t) st & 1) == 0);
            seL4_CPtr p = small_table_alloc(object_type);
            if (p == seL4_CapNull) {
                mem_fx_free(st, sizeof(struct small_tok));
                return NULL;
            }
            st->ptr = p;
            return (object_token) ((uintptr_t) st | 1);
        }
        default: {
            untyped_4k_ref ref = untyped_allocate_retyped(object_type);
            if (ref == NULL) {
                return NULL;
            }
            assert(((uintptr_t) ref & 1) == 0);
            return (object_token) ref;
        }
    }
}

seL4_CPtr object_cap(object_token token_r) {
    assert(token_r != NULL);
    if (((uintptr_t) token_r) & 1) {
        struct small_tok *st = (struct small_tok *) (((uintptr_t) token_r) & ~1);
        return st->ptr;
    } else {
        return untyped_auxptr_4k((untyped_4k_ref) token_r);
    }
}

void object_free_token(object_token token_r) {
    assert(token_r != NULL);
    if (((uintptr_t) token_r) & 1) {
        struct small_tok *st = (struct small_tok *) (((uintptr_t) token_r) & ~1);
        small_table_free(st->ptr);
        mem_fx_free(st, sizeof(struct small_tok));
    } else {
        untyped_free_4k((untyped_4k_ref) token_r);
    }
}
