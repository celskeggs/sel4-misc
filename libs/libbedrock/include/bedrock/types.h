#ifndef LIB_BEDROCK_TYPES_H
#define LIB_BEDROCK_TYPES_H

#include <sel4/simple_types.h>

#ifndef NULL
#define NULL ((void*) 0)
#endif

// just for liblua to not complain
#define LLONG_MAX (0x7FFFFFFFFFFFFFFFLL)
#define INTPTR_MAX (0x7FFFFFFF)
#define INT_MAX (0x7FFFFFFF)
#define UCHAR_MAX (0xFF)

typedef seL4_Uint8 uint8_t;
typedef seL4_Uint16 uint16_t;
typedef seL4_Uint32 uint32_t;
typedef seL4_Uint64 uint64_t;
typedef seL4_Int8 int8_t;
typedef seL4_Int16 int16_t;
typedef seL4_Int32 int32_t;
typedef seL4_Int64 int64_t;

// TODO: maybe verify this?
typedef seL4_Int32 sig_atomic_t;

// TODO: support 64-bit systems
typedef seL4_Int32 ssize_t;
typedef seL4_Uint32 size_t;
typedef seL4_Uint32 uintptr_t;
typedef seL4_Int32 intptr_t;
typedef seL4_Int32 ptrdiff_t;

typedef seL4_Int8 bool;
#define false 0
#define true 1

#define __FORCE_INCLUSION(x) void *__force_inclusion_ ## x = x;

#endif //LIB_BEDROCK_TYPES_H
