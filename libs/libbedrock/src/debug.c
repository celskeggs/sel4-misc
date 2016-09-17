#include <bedrock/debug.h>

void debug_print_raw(const char *str) {
    while (*str) {
        seL4_DebugPutChar(*str++);
    }
}

void debug_print(const char *str) {
    debug_print_raw("[DEBUG] ");
    debug_print_raw(str);
}

void debug_println(const char *str) {
    debug_print(str);
    seL4_DebugPutChar('\n');
}

void debug_printdec(int64_t value) {
    if (value == 0) {
        debug_println("0");
        return;
    }
    bool neg = (value < 0);
    if (neg) { value = -value; }
    char out[24];
    int i = sizeof(out);
    out[--i] = '\0';
    out[--i] = '\n';
    while (value > 0) {
        out[--i] = (char) ((value % 10) + '0');
        value /= 10;
    }
    if (neg) {
        out[--i] = '-';
    }
    debug_print(out + i);
}

void debug_printhex(uint64_t value) {
    if (value == 0) {
        debug_println("0");
        return;
    }
    char out[24];
    int i = sizeof(out);
    out[--i] = '\0';
    out[--i] = '\n';
    while (value > 0) {
        uint32_t v = (uint32_t) (value & 0xF);
        out[--i] = (char) (v < 10 ? v + '0' : v - 10 + 'A');
        value >>= 4;
    }
    out[--i] = 'x';
    out[--i] = '0';
    debug_print(out + i);
}

void debug_printregion(const char *name, seL4_SlotRegion region) {
    debug_println(name);
    debug_printdec(region.start);
    debug_printdec(region.end);
}