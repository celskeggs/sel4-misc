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

#include "serial.h"

/*
 * Port offsets
 * W    - write
 * R    - read
 * RW   - read and write
 * DLAB - Alternate register function bit
 */

#define SERIAL_THR  0 /* Transmitter Holding Buffer (W ) DLAB = 0 */
#define SERIAL_RBR  0 /* Receiver Buffer            (R ) DLAB = 0 */
#define SERIAL_DLL  0 /* Divisor Latch Low Byte     (RW) DLAB = 1 */
#define SERIAL_IER  1 /* Interrupt Enable Register  (RW) DLAB = 0 */
#define SERIAL_DLH  1 /* Divisor Latch High Byte    (RW) DLAB = 1 */
#define SERIAL_IIR  2 /* Interrupt Identification   (R ) */
#define SERIAL_FCR  2 /* FIFO Control Register      (W ) */
#define SERIAL_LCR  3 /* Line Control Register      (RW) */
#define SERIAL_MCR  4 /* Modem Control Register     (RW) */
#define SERIAL_LSR  5 /* Line Status Register       (R ) */
#define SERIAL_MSR  6 /* Modem Status Register      (R ) */
#define SERIAL_SR   7 /* Scratch Register           (RW) */
#define SERIAL_DLAB BIT(7)
#define SERIAL_LSR_DATA_READY BIT(0)
#define SERIAL_LSR_TRANSMITTER_EMPTY BIT(5)

#define IO_IN_8(device, ser) (seL4_IA32_IOPort_In8(device->io_cap, device->io_port + (ser)))
#define IO_OUT_8(device, ser, value) assert(!seL4_IA32_IOPort_Out8(device->io_cap, device->io_port + (ser), value))

int serial_getchar(serial_dev_t *device) {
    /* Check if character is available. */
    seL4_IA32_IOPort_In8_t in = IO_IN_8(device, SERIAL_LSR);
    assert(!in.error);
    if (!(in.result & SERIAL_LSR_DATA_READY)) {
        return -1;
    }

    /* retrieve character */
    in = IO_IN_8(device, SERIAL_RBR);
    assert(!in.error);

    return (int) in.result;
}

int serial_ready(serial_dev_t *device) {
    seL4_IA32_IOPort_In8_t in = IO_IN_8(device, SERIAL_LSR);
    assert(!in.error);
    return (in.result & SERIAL_LSR_TRANSMITTER_EMPTY) != 0;
}

int serial_putchar(serial_dev_t *device, char c) {
    /* Check if serial is ready. */
    if (!serial_ready(device)) {
        return -1;
    }
    /* Write out the next character. */
    IO_OUT_8(device, SERIAL_THR, (uint8_t) c);
    return 0;
}

ssize_t serial_write(serial_dev_t *d, const char *vdata, size_t count) {
    for (int i = 0; i < count; i++) {
        if (serial_putchar(d, *vdata++) < 0) {
            return i;
        }
    }
    return count;
}

ssize_t serial_read(serial_dev_t *d, char *buf, size_t count) {
    for (int i = 0; i < count; i++) {
        int ret = serial_getchar(d);
        if (ret == EOF) {
            return i;
        }
        *buf++ = (uint8_t) ret;
    }
    return count;
}

int serial_init(enum serial_port serial_port, seL4_IA32_IOPort io_cap, serial_dev_t *dev) {
    uint32_t io_port;
    switch (serial_port) {
        case SERIAL_COM1:
            io_port = 0x3f8;
            break;
        case SERIAL_COM2:
            io_port = 0x2f8;
            break;
        case SERIAL_COM3:
            io_port = 0x3e8;
            break;
        case SERIAL_COM4:
            io_port = 0x2e8;
            break;
        default:
            return 1;
    }
    dev->io_port = io_port;
    dev->io_cap = io_cap;

    /* clear DLAB - Divisor Latch Access Bit */
    IO_OUT_8(dev, SERIAL_LCR, 0x00);
    /* disable generating interrupts */
    IO_OUT_8(dev, SERIAL_IER, 0x00);

    /* set DLAB to*/
    IO_OUT_8(dev, SERIAL_LCR, SERIAL_DLAB);
    /* set low byte of divisor to 0x01 = 115200 baud */
    IO_OUT_8(dev, SERIAL_DLL, 0x01);
    /* set high byte of divisor to 0x00 */
    IO_OUT_8(dev, SERIAL_DLH, 0x00);

    /* line control register: set 8 bit, no parity, 1 stop bit; clear DLAB */
    IO_OUT_8(dev, SERIAL_LCR, 0x03 & ~SERIAL_DLAB);
    /* modem control register: set DTR/RTS/OUT2 */
    IO_OUT_8(dev, SERIAL_MCR, 0x0b);

    /* clear receiver port */
    seL4_IA32_IOPort_In8_t in = IO_IN_8(dev, SERIAL_RBR);
    assert(!in.error);
    /* clear line status port */
    in = IO_IN_8(dev, SERIAL_LSR);
    assert(!in.error);
    /* clear modem status port */
    in = IO_IN_8(dev, SERIAL_MSR);
    assert(!in.error);

    /* Enable the receiver interrupt. */
    IO_OUT_8(dev, SERIAL_IER, 0x01);
    return 0;
}
