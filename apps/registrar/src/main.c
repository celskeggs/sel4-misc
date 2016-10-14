#include <bedrock/debug.h>
#include <init/init.h>
#include <ipc/ipc.h>
#include <resource/mem_page.h>
#include <resource/mem_fx.h>
#include <resource/cslot.h>

// This server tracks a tree structure - like a filesystem - for registration of services.

// makes a directed acyclic graph with exactly one reference to each node.
struct node {
    uint32_t badge_number; // or "0" for "external capability"
    seL4_CPtr capability; // either an external capability or the badged original endpoint
    struct node *first_child;
    struct node *sibling;
};

static struct node *root = NULL;
static struct node *free_nodes = NULL;
static seL4_CPtr registrar_endpoint;

static uint32_t next_badge_number = 1;

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

static struct node *leaf_new(void) {
    struct node *out = mem_fx_alloc(sizeof(struct node));
    if (out == NULL) {
        ERRX_TRACEPOINT;
        return NULL;
    }
    out->capability = cslot_alloc();
    if (out->capability == seL4_CapNull) {
        mem_fx_free(out, sizeof(struct node));
        ERRX_TRACEPOINT;
        return NULL;
    }
    out->badge_number = 0;
    out->first_child = NULL;
    out->sibling = NULL;
    return out;
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

bool main(void) {
    debug_println("registrar init...");

    object_token ep = object_alloc(seL4_EndpointObject);
    registrar_endpoint = object_cap(ep);
    (void) ep; // will never be freed.

    root = node_new();

    struct ipc_out_sandbox_ready out;
    if (!ipc_sandbox_ready(ecap_IOEP, registrar_endpoint, &(struct ipc_in_sandbox_ready) {}, &out)) {
        ERRX_TRACEPOINT;
        return false;
    }

    DEBUG("ready signal sent. serving now.");

    SERVER_LOOP(registrar_endpoint, true, SERVER_FOR(ping))
}
