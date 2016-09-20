#include <bedrock/assert.h>
#include <bedrock/debug.h>

void _assert_fail_static(const char *fail) {
    _assert_fail_static_2("", fail);
}

void _assert_fail_static_2(const char *fail_1, const char *fail_2) {
#ifdef SEL4_DEBUG_KERNEL
    debug_print(fail_1);
    debug_println(fail_2);
    seL4_DebugHalt();
#endif
    __builtin_trap();
    while (1) {
        // loop forever
    }
}
