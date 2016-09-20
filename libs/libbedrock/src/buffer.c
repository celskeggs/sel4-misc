#include <bedrock/buffer.h>
#include <bedrock/assert.h>

void *memset(void *buf, int c, size_t count) {
    assert(buf != NULL);
    uint8_t *u8b = (uint8_t *) buf;
    for (size_t i = 0; i < count; i++) {
        u8b[i] = (uint8_t) c;
    }
    return buf;
}

void *memcpy(void *dest, const void *src, size_t count) {
    assert(dest != NULL && src != NULL);
    uint8_t *u8d = (uint8_t *) dest;
    uint8_t *u8s = (uint8_t *) src;
    for (size_t i = 0; i < count; i++) {
        u8d[i] = u8s[i];
    }
    return dest;
}

// moves as much as possible of a string into a buffer, but makes sure the result is always property terminated.
extern char *strblit(char *dest, size_t buffer_len, const char *src) {
    assert(dest != NULL && src != NULL);
    size_t i;
    for (i = 0; i < buffer_len - 1 && *src; i++) {
        dest[i] = *src;
    }
    assert(0 <= i && i < buffer_len);
    dest[i] = '\0'; // always null-terminated
    return dest;
}

size_t strlen(const char *ptr) {
    const char *cur = ptr;
    while (*cur) {
        cur++;
    }
    return cur - ptr;
}
