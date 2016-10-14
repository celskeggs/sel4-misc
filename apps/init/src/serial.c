#include "serial.h"
#include <resource/cslot.h>
#include <resource/object.h>

// #define SERIAL_IO_PORT 0x3F8

#define SERIAL_IO_DATA (SERIAL_IO_PORT + 0)
#define SERIAL_IO_INTERRUPT_ENABLE (SERIAL_IO_PORT + 1)
#define SERIAL_IO_DIVISOR_LSB (SERIAL_IO_PORT + 0)
#define SERIAL_IO_DIVISOR_MSB (SERIAL_IO_PORT + 1)
#define SERIAL_IO_FIFO_CTRL_REG (SERIAL_IO_PORT + 2)
#define SERIAL_IO_LINE_CTRL_REG (SERIAL_IO_PORT + 3)
#define SERIAL_IO_MODEM_CTRL_REG (SERIAL_IO_PORT + 4)
#define SERIAL_IO_LINE_STAT_REG (SERIAL_IO_PORT + 5)
#define SERIAL_IO_MODEM_STAT_REG (SERIAL_IO_PORT + 6)
#define SERIAL_IO_SCRATCH_REG (SERIAL_IO_PORT + 7)

#define LINE_CTRL_8_DATA_BITS ((uint8_t) 0x03)
#define LINE_CTRL_1_STOP_BIT ((uint8_t) 0x00)
#define LINE_CTRL_NO_PARITY ((uint8_t) 0x00)
#define LINE_CTRL_DLAB ((uint8_t) 0x80)

#define FIFO_CTRL_ENABLE_FIFO ((uint8_t) 0x01)
#define FIFO_CTRL_CLEAR_FIFOS ((uint8_t) 0x06)
#define FIFO_CTRL_THRESHOLD_14B ((uint8_t) 0xC0)

#define MODEM_CTRL_FORCE_DTR ((uint8_t) 0x01)
#define MODEM_CTRL_FORCE_RTS ((uint8_t) 0x02)
#define MODEM_CTRL_AUX_2 ((uint8_t) 0x08)

#define LINE_STAT_OKAY_TO_WRITE ((uint8_t) 0x20)

static seL4_IA32_IOPort io = seL4_CapNull;
static seL4_IRQHandler handler = seL4_CapNull;
static seL4_CPtr notification = seL4_CapNull;
static serial_cb read_callback = NULL;

#define IOACCESS seL4_Error err; seL4_IA32_IOPort_In8_t ret; // used for the next two definitions
#define OUT(port, value) if ((err = seL4_IA32_IOPort_Out8(io, port, value)) != seL4_NoError) { ERRX_RAISE_SEL4(err); return false; }
#define IN(port) if ((ret = seL4_IA32_IOPort_In8(io, port)).error != seL4_NoError) { ERRX_RAISE_SEL4(ret.error); return false; }
#define INVALUE (ret.result)

bool serial_init(seL4_IA32_IOPort iop, seL4_IRQControl ctrl, serial_cb cb) {
    io = iop;
    IOACCESS
    handler = cslot_alloc();
    if (handler == seL4_CapNull) {
        ERRX_TRACEPOINT;
        return false;
    }
    object_token token = object_alloc(seL4_EndpointObject);
    if (token == NULL) {
        ERRX_TRACEPOINT;
        return false;
    }
    notification = object_cap(token);
    if (!cslot_irqget(ctrl, SERIAL_IO_IRQ, handler)) {
        ERRX_TRACEPOINT;
        return false;
    }
    assert(seL4_IRQHandler_SetNotification(handler, notification) == seL4_NoError);
    OUT(SERIAL_IO_INTERRUPT_ENABLE, 0x00);
    OUT(SERIAL_IO_LINE_CTRL_REG, LINE_CTRL_DLAB);
    // 0x0001: divisor of 1, for 115200 baud
    OUT(SERIAL_IO_DIVISOR_MSB, 0x00);
    OUT(SERIAL_IO_DIVISOR_LSB, 0x01);
    OUT(SERIAL_IO_LINE_CTRL_REG, LINE_CTRL_8_DATA_BITS | LINE_CTRL_1_STOP_BIT | LINE_CTRL_NO_PARITY);
    OUT(SERIAL_IO_FIFO_CTRL_REG, FIFO_CTRL_ENABLE_FIFO | FIFO_CTRL_CLEAR_FIFOS | FIFO_CTRL_THRESHOLD_14B);
    OUT(SERIAL_IO_MODEM_CTRL_REG, MODEM_CTRL_FORCE_DTR | MODEM_CTRL_FORCE_RTS | MODEM_CTRL_AUX_2);
    read_callback = cb;
    OUT(SERIAL_IO_INTERRUPT_ENABLE, 0x1);
    IN(SERIAL_IO_LINE_STAT_REG);
    assert(!(INVALUE & 0x01));
    // TODO: actually READ stuff
    return true;
}

void serial_wait_ready(void) {
    IOACCESS
    while (1) {
        assert(seL4_IRQHandler_Ack(handler) == seL4_NoError);
        DEBUG("waiting...");
        ret = seL4_IA32_IOPort_In8(io, SERIAL_IO_LINE_STAT_REG);
        assert(ret.error == seL4_NoError);
        if (!(INVALUE & 0x01)) {
            DEBUG("blank");
        } else {
            ret = seL4_IA32_IOPort_In8(io, SERIAL_IO_DATA);
            assert(ret.error == seL4_NoError);
            debug_printhex(INVALUE);
        }
        seL4_Wait(notification, NULL);
    }
}

bool serial_write_byte(char b) {
    IOACCESS
    IN(SERIAL_IO_LINE_STAT_REG);
    assert(!(INVALUE & 0x01));
    if ((INVALUE & LINE_STAT_OKAY_TO_WRITE) == 0) {
        ERRX_RAISE_GENERIC(GERR_DATA_SPILLED);
        return false; // TODO: actually respond to this properly. we can't test this on qemu, though.
    }
    OUT(SERIAL_IO_DATA, (uint8_t) b);
    return true;
}

bool serial_write(char *data, size_t length) {
    assert(io != seL4_CapNull);
    assert(data != NULL);
    while (length-- > 0) {
        if (!serial_write_byte(*data++)) {
            return false;
        }
    }
    return true;
}
