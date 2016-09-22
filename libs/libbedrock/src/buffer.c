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
// returns a pointer to the generated null terminator
extern char *strblit(char *dest, size_t buffer_len, const char *src) {
    assert(dest != NULL && src != NULL);
    size_t i;
    for (i = 0; i < buffer_len - 1 && src[i]; i++) {
        dest[i] = src[i];
    }
    assert(i < buffer_len);
    dest[i] = '\0'; // always null-terminated
    return &dest[i];
}

// same as strblit, but takes buffer_len as a pointer and updates it.
// used for successive blits.
extern char *strblitadv(char *dest, size_t *buffer_len_p, const char *src) {
    assert(dest != NULL && src != NULL);
    size_t buffer_len = *buffer_len_p;
    size_t i;
    for (i = 0; i < buffer_len - 1 && src[i]; i++) {
        dest[i] = src[i];
    }
    assert(i < buffer_len);
    dest[i] = '\0'; // always null-terminated
    *buffer_len_p = buffer_len - i;
    return &dest[i];
}

size_t strlen(const char *ptr) {
    const char *cur = ptr;
    while (*cur) {
        cur++;
    }
    return cur - ptr;
}
