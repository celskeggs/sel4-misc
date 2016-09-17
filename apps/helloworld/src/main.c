#include <sel4/sel4.h>
#include <bedrock/debug.h>

void premain(void) {
    debug_println("hello, world!");

#ifdef SEL4_DEBUG_KERNEL
    seL4_DebugHalt();
#endif
    __builtin_trap();
    while (1) {
        // do nothing
    }
}
