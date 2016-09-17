/*
 * Copyright 2014, NICTA
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(NICTA_BSD)
 */

#define __ASM__
#include <sel4/arch/constants.h>

    .global _start
    .text

_start:
    leal _stack_top, %esp

    /* Setup segment selector for IPC buffer access. */
    movw $IPCBUF_GDT_SELECTOR, %ax
    movw %ax, %gs

    /* Start main. */
    pushl %ebx
    call premain
_fail:
    jmp _fail

    .bss
    .align 4

_stack_bottom:
    .space 65536
_stack_top: