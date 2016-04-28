/*
 * Copyright 2016 Cel Skeggs
 * Based on code:
 * Copyright 2014, NICTA
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(NICTA_BSD)
 */
#ifndef _PLATSUPPORT_PLAT_SERIAL_H
#define _PLATSUPPORT_PLAT_SERIAL_H

#include "../basic.h"

typedef struct serial_dev {
    uint32_t io_port;
    seL4_IA32_IOPort io_cap;
} serial_dev_t;

enum serial_port {
    SERIAL_COM1,
    SERIAL_COM2,
    SERIAL_COM3,
    SERIAL_COM4
};

int serial_init(enum serial_port io_port, seL4_IA32_IOPort io_cap, serial_dev_t* dev);
ssize_t serial_read(serial_dev_t* d, char* vdata, size_t count);
ssize_t serial_write(serial_dev_t* d, const char* vdata, size_t count);
int serial_putchar(serial_dev_t* device, char c);
int serial_ready(serial_dev_t* device);
int serial_getchar(serial_dev_t *device);

#endif /* _PLATSUPPORT_PLAT_SERIAL_H */
