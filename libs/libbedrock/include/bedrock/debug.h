#ifndef LIB_BEDROCK_DEBUG_H
#define LIB_BEDROCK_DEBUG_H

#include <sel4/sel4.h>
#include "types.h"

#ifdef SEL4_DEBUG_KERNEL

extern void debug_print_raw(const char *str);

extern void debug_print(const char *str);

extern void debug_println(const char *str);

extern void debug_printdec(int64_t value);

extern void debug_printhex(uint64_t value);

extern void debug_printregion(const char *name, seL4_SlotRegion region);

#endif

#ifdef SEL4_DEBUG_KERNEL
#define _DEBUG_INTERNAL(text, file, line) debug_println(file ":" _fail_tostring(line) ": " text)
#define DEBUG(x) (_DEBUG_INTERNAL(x, __FILE__, __LINE__))
#else
#define DEBUG(x)
#endif

#endif //LIB_BEDROCK_DEBUG_H
