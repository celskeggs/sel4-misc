#ifndef SEL4_MISC_SERIAL_H
#define SEL4_MISC_SERIAL_H

#include <bedrock/bedrock.h>
#include <bedrock/errx.h> // this module uses errx

// only supports serial port COM1 for now
#define SERIAL_IO_PORT 0x3F8
#define SERIAL_IO_IRQ 4

typedef void (*serial_cb)(uint8_t byte);

bool serial_init(seL4_IA32_IOPort port, seL4_IRQControl ctrl, serial_cb callback);
bool serial_write(char *data, size_t length);

// temp
void serial_wait_ready(void);

#endif //SEL4_MISC_SERIAL_H
