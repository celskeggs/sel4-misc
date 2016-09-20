#ifndef LIB_BEDROCK_ASSERT_H
#define LIB_BEDROCK_ASSERT_H

extern void _assert_fail_static(const char *fail) __attribute__((noreturn));
extern void _assert_fail_static_2(const char *fail_1, const char *fail_2) __attribute__((noreturn));

#define _fail_tostring(x) #x

#define _assert_fail(expr, file, line) _assert_fail_static(file ":" _fail_tostring(line) ": assertion '" expr "' failed.")
#define assert(expr) (expr ? ((void) 0) : _assert_fail(#expr, __FILE__, __LINE__))

#define _fail_fail(mesg, file, line) _assert_fail_static_2(file ":" _fail_tostring(line) ": ", mesg)
#define fail(mesg) (_fail_fail(mesg, __FILE__, __LINE__))

#endif //LIB_BEDROCK_ASSERT_H
