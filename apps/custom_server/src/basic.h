#ifndef SEL4_MISC_BASIC_H
#define SEL4_MISC_BASIC_H

#include <sel4/sel4.h>

#define NULL ((void*) 0)
#define EOF (-1)

typedef seL4_Uint8 uint8_t;
typedef seL4_Uint16 uint16_t;
typedef seL4_Uint32 uint32_t;
typedef seL4_Uint64 uint64_t;
typedef seL4_Int8 int8_t;
typedef seL4_Int16 int16_t;
typedef seL4_Int32 int32_t;
typedef seL4_Int64 int64_t;

// TODO: calculate this better
typedef seL4_Int32 ssize_t;
typedef seL4_Uint32 size_t;
typedef seL4_Uint32 uintptr_t;

static inline void *memset(void *buf, int c, size_t count) {
    uint8_t *u8b = (uint8_t *) buf;
    for (size_t i = 0; i < count; i++) {
        u8b[i] = (uint8_t) c;
    }
    return buf;
}

static inline void debug_print_raw(const char *str) {
    while (*str) {
        seL4_DebugPutChar(*str++);
    }
}

static inline void debug_print(const char *str) {
    debug_print_raw("[DEBUG] ");
    debug_print_raw(str);
}

static inline void debug_println(const char *str) {
    debug_print(str);
    seL4_DebugPutChar('\n');
}

#define _assert_fail_tostring(x) #x
#define _assert_fail(expr, file, line) _assert_fail_static(file ":" _assert_fail_tostring(line) ": assertion '" expr "' failed.")
#define assert(expr) (expr ? ((void) 0) : _assert_fail(#expr, __FILE__, __LINE__))

#endif //SEL4_MISC_BASIC_H
