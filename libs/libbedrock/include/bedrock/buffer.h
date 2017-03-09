#ifndef LIB_BEDROCK_BUFFER_H
#define LIB_BEDROCK_BUFFER_H

#include "types.h"

extern void *memset(void *buf, int c, size_t count);

extern void *memcpy(void *dest, const void *src, size_t count);

extern char *strblit(char *dest, size_t buffer_len, const char *src);

extern char *strblitadv(char *dest, size_t *buffer_len, const char *src);

extern size_t strlen(const char *ptr);

extern bool streq(const char *a, const char *b);

extern int strcmp(const char *a, const char *b);

extern int strncmp(const char *a, const char *b, size_t count);

extern bool strstart(const char *shorter, const char *longer);

extern const char *strstr(const char *haystack, const char *needle);

extern const char *strchr(const char *s, int c);

extern size_t strspn(const char *s, const char *accept);

extern bool isalpha(int c);

extern bool isalnum(int c);

extern bool isdigit(int c);

extern bool isspace(int c);

extern int toupper(int c);

#endif //LIB_BEDROCK_BUFFER_H
