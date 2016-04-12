/*
 * Copyright 2014, NICTA
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(NICTA_BSD)
 */

#include <sel4/sel4.h>

int
_begin(void)
{
    char *str = "[CEL'S STUFF] Hello, seL4 World!\n";
    while (*str) {
        seL4_DebugPutChar(*str++);
    }
    seL4_DebugHalt();

    while (1) {
        // do nothing
    }
}
