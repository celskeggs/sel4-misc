#ifndef LIB_BEDROCK_TYPES_H
#define LIB_BEDROCK_TYPES_H

#include <sel4/simple_types.h>

#define NULL ((void*) 0)

typedef seL4_Uint8 uint8_t;
typedef seL4_Uint16 uint16_t;
typedef seL4_Uint32 uint32_t;
typedef seL4_Uint64 uint64_t;
typedef seL4_Int8 int8_t;
typedef seL4_Int16 int16_t;
typedef seL4_Int32 int32_t;
typedef seL4_Int64 int64_t;

// TODO: support 64-bit systems
typedef seL4_Int32 ssize_t;
typedef seL4_Uint32 size_t;
typedef seL4_Uint32 uintptr_t;

typedef seL4_Int8 bool;
#define false 0
#define true 1

#endif //LIB_BEDROCK_TYPES_H
