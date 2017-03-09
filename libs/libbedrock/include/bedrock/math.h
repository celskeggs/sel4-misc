#ifndef LIB_BEDROCK_MATH_H
#define LIB_BEDROCK_MATH_H

// derived from musl code

extern double pow(double, double);

extern double floor(double);

extern double fmod(double, double);

// not derived from musl code

extern double str10tod(const char *, const char **);

extern int abs(int);

extern double fabs(double);

#endif //LIB_BEDROCK_MATH_H
