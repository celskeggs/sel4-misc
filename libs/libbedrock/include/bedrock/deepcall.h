#ifndef LIB_BEDROCK_SETJMP_H
#define LIB_BEDROCK_SETJMP_H

#include "types.h"

// return true if returned normally; false if deep_return was used
extern bool deep_call(void (*target)(void *), void *param, void **saveaddr);
extern void deep_return(void *loadaddr) __attribute__((noreturn));

#endif //LIB_BEDROCK_SETJMP_H
