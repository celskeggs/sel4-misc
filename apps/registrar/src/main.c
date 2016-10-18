#include <bedrock/debug.h>
#include <init/init.h>
#include <ipc/ipc.h>
#include <resource/mem_page.h>
#include <resource/mem_fx.h>
#include <resource/cslot.h>
#include "hashmap.h"

// This server tracks a linked hashmap mapping registration paths ("dev.ser0") to capabilities for registered services.

// potential access rights:
// * list or retrieve registrations within a prefix
// * store registrations within a prefix
// * store a particular registration

struct registration {
    char *name;
    seL4_CPtr capability;
    struct registration *prev;
    struct registration *next;
};

#define ACCESSOR_COUNT 1024

struct accessor {
    char *prefix;
    // handles both badge_number+0 (as read+write) and badge_number+1 (as read only)
    uint32_t badge_number; // lowest bit is unset.
    struct accessor *next;
};

static struct registration *registration_first = NULL; // first in terms of ordering
// map of registration paths -> capabilities.
static struct hashmap *registration_map = NULL;
// map of badge numbers -> accessors. based on lowest bits (except very lowest); accessor->next is used for chaining.
static struct accessor *accessor_map[ACCESSOR_COUNT] = {NULL};

static seL4_CPtr registrar_endpoint;

static uint32_t next_badge_number = 2;

static struct accessor *get_accessor_by_badge(uint32_t badge_number) {
    badge_number >>= 1; // ignore readonly bit
    struct accessor *accessor = accessor_map[badge_number & (ACCESSOR_COUNT - 1)];
    assert(accessor != NULL); // in theory, MUST exist! otherwise there's a security flaw...
    while (accessor->badge_number != badge_number) {
        accessor = accessor->next;
        assert(accessor != NULL); // in theory, MUST exist! otherwise there's a security flaw...
    }
    return accessor;
}

static struct node *node_new(void) {
    if (free_nodes != NULL) {
        struct node *out = free_nodes;
        free_nodes = free_nodes->sibling;
        assert(out->badge_number != 0); // badge number is preserved.
        assert(out->capability == seL4_CapNull);
        out->sibling = out->first_child = NULL;
        return out;
    }
    if (next_badge_number == 0) {
        // this is an okay place to error because it means we're currently using 2^32 - 1 badges, and that's VERY UNLIKELY.
        ERRX_RAISE_GENERIC(GERR_MEMORY_POOL_EXHAUSTED);
        return NULL;
    }
    struct node *node = mem_fx_alloc(sizeof(struct node));
    if (node == NULL) {
        ERRX_TRACEPOINT;
        return NULL;
    }
    node->capability = cslot_alloc();
    if (node->capability == seL4_CapNull) {
        mem_fx_free(node, sizeof(struct node));
        ERRX_TRACEPOINT;
        return NULL;
    }
    node->badge_number = next_badge_number++;
    if (!cslot_mint(registrar_endpoint, node->capability, node->badge_number)) {
        cslot_free(node->capability);
        mem_fx_free(node, sizeof(struct node));
        ERRX_TRACEPOINT;
        return NULL;
    }
    node->first_child = NULL;
    node->sibling = NULL;
    return node;
}

static void node_recycle(struct node *node) {
    assert(node->sibling == NULL && node->first_child == NULL);
    if (node->badge_number == 0) {
        cslot_free(node->capability);
        mem_fx_free(node, sizeof(struct node));
    } else {
        // gets rid of any child caps, so that badge number reuse isn't a problem.
        assert(cslot_recycle(node->capability));
        // TODO: ensure that this doesn't interfere with other nodes on this registrar
        // TODO: ensure that access is properly revoked
        node->sibling = free_nodes;
        free_nodes = node;
    }
}

bool ipc_handle_ping(uint32_t sender, struct ipc_in_ping *in, struct ipc_out_ping *out) {
    out->value_neg = -(in->value);
    DEBUG("responding to ping");
    return true;
}

bool ipc_handle_registrar_iter_first(uint32_t sender, struct ipc_in_registrar_iter_first *in,
                                     struct ipc_out_registrar_iter_first *out) {
    struct accessor *accessor = get_accessor_by_badge(sender);
    struct registration *reg = registration_first;
    while (reg != NULL) { // TODO: optimize this searching algorithm
        if (strstart(accessor->prefix, reg->name)) {
            if (!pack_ipc_string(reg->name + strlen(accessor->prefix), out->name)) {
                ERRX_TRACEPOINT;
                return false;
            }
            out->any = true;
            return true;
        }
        reg = reg->next;
    }
    memset(out->name, 0, IPC_STR_LEN);
    out->any = false;
    return true;
}

bool ipc_handle_registrar_iter_next(uint32_t sender, struct ipc_in_registrar_iter_next *in,
                                    struct ipc_out_registrar_iter_next *out) {
    struct accessor *accessor = get_accessor_by_badge(sender);
    const char *key = unpack_ipc_string(in->name);
    if (key == NULL) {
        ERRX_TRACEPOINT;
        return false;
    }
    char composite_key[strlen(key) + strlen(accessor->prefix) + 1];
    strcpy(composite_key, accessor->prefix);
    strcpy(composite_key + strlen(accessor->prefix), key);
    free_ipc_string(key);
    struct registration *reg = hashmap_get(registration_map, composite_key);
    if (reg == NULL) {
        ERRX_RAISE_GENERIC(GERR_DATA_SPILLED);
        return false;
    }
    reg = reg->next;
    if (reg != NULL && strstart(accessor->prefix, reg->name)) {
        if (!pack_ipc_string(reg->name + strlen(accessor->prefix), out->name)) {
            ERRX_TRACEPOINT;
            return false;
        }
        // if the name is empty, it'll show up as END OF STREAM to the client. this should never happen here, but let's assert that.
        assert(out->name[0]);
        return true;
    } else {
        memset(out->name, 0, IPC_STR_LEN);
        return true;
    }
}

bool ipc_handle_registrar_derive(uint32_t sender, seL4_CPtr cap_out, struct ipc_in_registrar_derive *in,
                                 struct ipc_out_registrar_derive *out) {

}

bool ipc_handle_registrar_lookup(uint32_t sender, seL4_CPtr cap_out, struct ipc_in_registrar_lookup *in,
                                 struct ipc_out_registrar_lookup *out) {
    struct accessor *accessor = get_accessor_by_badge(sender);
    const char *key = unpack_ipc_string(in->name);
    if (key == NULL) {
        ERRX_TRACEPOINT;
        return false;
    }
    char composite_key[strlen(key) + strlen(accessor->prefix) + 1];
    strcpy(composite_key, accessor->prefix);
    strcpy(composite_key + strlen(accessor->prefix), key);
    free_ipc_string(key);
    struct registration *reg = hashmap_get(registration_map, composite_key);
    if (reg == NULL) {
        ERRX_RAISE_GENERIC(GERR_TARGET_NOT_FOUND);
        return false;
    }
    if (!cslot_copy(reg->capability, cap_out)) {
        ERRX_TRACEPOINT;
        return false;
    }
    return true;
}

static void join_registration_into_linked_list(struct registration *reg) {
    if (registration_first == NULL) {
        registration_first = reg;
        reg->next = reg->prev = NULL;
        return;
    }
    struct registration *ecur = registration_first;
    assert(ecur->prev == NULL);
    if (strcmp(reg->name, ecur->name) < 0) {
        reg->prev = NULL;
        reg->next = registration_first;
        registration_first->prev = reg;
        registration_first = reg;
        return;
    }
    while (strcmp(reg->name, ecur->name) > 0) {
        assert(strcmp(reg->name, ecur->name) != 0);
        if (ecur->next == NULL) {
            // reached end
            reg->prev = ecur;
            reg->next = NULL;
            ecur->next = reg;
            return;
        }
        ecur = ecur->next;
    }
    reg->prev = ecur->prev;
    reg->next = ecur;
    reg->prev->next = reg;
    reg->next->prev = reg;
}

bool ipc_handle_registrar_register(uint32_t sender, seL4_CPtr cap_in, struct ipc_in_registrar_register *in,
                                   struct ipc_out_registrar_register *out) {
    struct accessor *accessor = get_accessor_by_badge(sender);
    if (sender & 1) { // attempt to make modification via read-only cap
        ERRX_RAISE_GENERIC(GERR_ACCESS_VIOLATION);
        return false;
    }
    const char *key = unpack_ipc_string(in->name);
    if (key == NULL) {
        ERRX_TRACEPOINT;
        return false;
    }
    char composite_key[strlen(key) + strlen(accessor->prefix) + 1];
    strcpy(composite_key, accessor->prefix);
    strcpy(composite_key + strlen(accessor->prefix), key);
    free_ipc_string(key);

    struct registration *reg = hashmap_get(registration_map, composite_key);
    if (reg != NULL) {
        ERRX_RAISE_GENERIC(GERR_UNSATISFIED_CONSTRAINT);
        return false;
    }
    reg = mem_fx_alloc(sizeof(struct registration));
    if (reg == NULL) {
        ERRX_TRACEPOINT;
        return false;
    }
    reg->capability = cslot_alloc();
    if (reg->capability == seL4_CapNull) {
        mem_fx_free(reg, sizeof(struct registration));
        ERRX_TRACEPOINT;
        return false;
    }
    if (!cslot_copy(reg->capability, cap_in)) {
        cslot_free(reg->capability);
        mem_fx_free(reg, sizeof(struct registration));
        ERRX_TRACEPOINT;
        return false;
    }
    reg->name = mem_fx_alloc(strlen(composite_key));
    if (reg->name == NULL) {
        cslot_free(reg->capability);
        mem_fx_free(reg, sizeof(struct registration));
        ERRX_TRACEPOINT;
        return false;
    }
    strcpy(reg->name, composite_key);
    void *last = NULL;
    if (!hashmap_set(registration_map, reg->name, reg, &last)) {
        mem_fx_free(reg->name, strlen(composite_key));
        cslot_free(reg->capability);
        mem_fx_free(reg, sizeof(struct registration));
        ERRX_TRACEPOINT;
        return false;
    }
    assert(last == NULL);
    join_registration_into_linked_list(reg);
    return true;
}

bool ipc_handle_registrar_remove(uint32_t sender, struct ipc_in_registrar_remove *in,
                                 struct ipc_out_registrar_remove *out) {

}

bool main(void) {
    debug_println("registrar init...");

    object_token ep = object_alloc(seL4_EndpointObject);
    if (ep == NULL) {
        ERRX_TRACEPOINT;
        return false;
    }
    registrar_endpoint = object_cap(ep);
    (void) ep; // will never be freed.

    registration_map = hashmap_new();

    struct ipc_out_sandbox_set_registrar out;
    if (!ipc_sandbox_set_registrar(ecap_IOEP, registrar_endpoint, &(struct ipc_in_sandbox_set_registrar) {}, &out)) {
        ERRX_TRACEPOINT;
        return false;
    }

    DEBUG("ready signal sent. serving now.");

    SERVER_LOOP(registrar_endpoint, true, SERVER_FOR(ping)
            SERVER_FOR(registrar_iter_first)
            SERVER_FOR(registrar_iter_next)
            SERVER_FOR(registrar_derive)
            SERVER_FOR(registrar_lookup)
            SERVER_FOR(registrar_register)
            SERVER_FOR(registrar_remove))
}
