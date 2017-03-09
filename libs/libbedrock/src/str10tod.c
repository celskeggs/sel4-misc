
#include <bedrock/types.h>
#include <bedrock/math.h>
#include <bedrock/buffer.h>

// for use in lua
double str10tod(const char *nptr, const char **endptr) { // TODO: handle overflows?
    const char *orig = nptr;
    while (isspace(*nptr)) {
        nptr++;
    }
    bool negate = false;
    if (*nptr == '+') {
        nptr++;
    } else if (*nptr == '-') {
        negate = true;
        nptr++;
    }
    double value = 0.0;
    bool any = false;
    while ('0' <= *nptr && *nptr <= '9') {
        any = true;
        value = (value * 10) + (*nptr++ - '0'); // TODO: maybe do this more accurately?
    }
    if (!any) {
        *endptr = orig;
        return 0;
    }
    if (*nptr == '.') {
        nptr++;
        double elem = 0.1;
        while ('0' <= *nptr && *nptr <= '9') {
            value += elem * (*nptr++ - '0');
            elem *= 0.1;
        }
    }
    if (*nptr == 'e' || *nptr == 'E') {
        const char *before_exp = nptr;
        nptr++;
        bool expneg = false;
        if (*nptr == '+') {
            nptr++;
        } else if (*nptr == '-') {
            nptr++;
            expneg = true;
        }
        int64_t exp = 0;
        any = false;
        while ('0' <= *nptr && *nptr <= '9') {
            any = true;
            exp = (exp * 10) + (*nptr - '0');
        }
        if (any) {
            if (exp != 0) {
                value *= pow(10, expneg ? -exp : exp);
            }
        } else {
            nptr = before_exp;
        }
    }
    if (endptr != NULL) {
        *endptr = nptr;
    }
    return value;
}