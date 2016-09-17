#include <bedrock/buffer.h>

void *memset(void *buf, int c, size_t count) {
    uint8_t *u8b = (uint8_t *) buf;
    for (size_t i = 0; i < count; i++) {
        u8b[i] = (uint8_t) c;
    }
    return buf;
}

void *memcpy(void *dest, const void *src, size_t count) {
    uint8_t *u8d = (uint8_t *) dest;
    uint8_t *u8s = (uint8_t *) src;
    for (size_t i = 0; i < count; i++) {
        u8d[i] = u8s[i];
    }
    return dest;
}

size_t strlen(const char *ptr) {
    const char *cur = ptr;
    while (*cur) {
        cur++;
    }
    return cur - ptr;
}
