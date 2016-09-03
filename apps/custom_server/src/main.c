#include <sel4/sel4.h>
#include "pc99/serial.h"
#include "alloc/cslot_ao.h"
#include "alloc/untyped.h"
#include "alloc/object.h"
#include "errno.h"
#include "alloc/mem_page.h"

void _assert_fail_static(const char *fail) {
    debug_println(fail);
    seL4_DebugHalt();
    while (1) {
        // loop forever
    }
}

static void print_range(const char *name, seL4_SlotRegion region) {
    debug_println(name);
    debug_printdec(region.start);
    debug_printdec(region.end);
}

void main(void) {
    DEX_START(dex1)
    assert(mem_page_map((void *) 0x400000, &dex1) == seL4_NoError);

    const char *source = "Hello, buffer world!";
    char *dest = (void *) 0x400000;
    do {
        *dest++ = *source;
    } while (*source++);
    debug_println((void *) 0x400000);

    DEX_END(dex1)

    serial_dev_t stdout;
    serial_init(SERIAL_COM1, seL4_CapIOPort, &stdout);
    serial_write(&stdout, "Hello, serial world!\n", (size_t) 21);

    debug_printdec(-1145435098);
    debug_printhex(0xAE871BC123CABEF0);
}

void _begin(seL4_BootInfo *bi) {
    seL4_InitBootInfo(bi);
    debug_println("Hello, seL4 World!");

    print_range("userImageFrames", bi->userImageFrames);
    print_range("userImagePTs", bi->userImagePTs);
    print_range("userImagePDs", bi->userImagePDs);
    cslot_ao_setup(bi->empty.start, bi->empty.end);

    assert(untyped_add_boot_memory(bi) == seL4_NoError);

    main();

    seL4_DebugHalt();

    while (1) {
        // do nothing
    }
}
