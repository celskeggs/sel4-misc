#include <bedrock/assert.h>
#include <bedrock/debug.h>

void _assert_fail_static(const char *fail) {
#ifdef SEL4_DEBUG_KERNEL
    debug_println(fail);
    seL4_DebugHalt();
#endif
    __builtin_trap();
    while (1) {
        // loop forever
    }
}
