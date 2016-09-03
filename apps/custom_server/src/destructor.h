#ifndef SEL4_MISC_DESTRUCTOR_H
#define SEL4_MISC_DESTRUCTOR_H

#include "basic.h"

#define DESTRUCTOR_COUNT 16

typedef void (*destructor_cb)(void *data);

struct destructor_ctx {
    struct destructor_ctx *next_ctx;
    uint8_t next_id;
    destructor_cb destructor_cbs[DESTRUCTOR_COUNT];
    void *destructor_data[DESTRUCTOR_COUNT];
};

typedef struct destructor_ctx *DEX;

#define DEX_START(name) struct destructor_ctx name = {.next_ctx = NULL, .next_id = 0};
#define DEX_END(name) destructor_destruct(&name);

void destructor_destruct(struct destructor_ctx *ctx);

bool destructor_insert(struct destructor_ctx *ctx, destructor_cb cb, void *data);

#endif //SEL4_MISC_DESTRUCTOR_H
