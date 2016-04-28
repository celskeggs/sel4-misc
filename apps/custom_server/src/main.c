#include <sel4/sel4.h>
#include "pc99/serial.h"

void _assert_fail_static(const char *fail) {
    debug_println(fail);
    seL4_DebugHalt();
    while (1) {
        // loop forever
    }
}

int _begin(void) {
    debug_println("Hello, seL4 World!");

    serial_dev_t stdout;
    serial_init(SERIAL_COM1, seL4_CapIOPort, &stdout);
    serial_write(&stdout, "Hello, serial world!\n", (size_t) 21);

    seL4_DebugHalt();

    while (1) {
        // do nothing
    }
}
