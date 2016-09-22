#include <sel4/assert.h>
#include <bedrock/errx.h>
#include <bedrock/buffer.h>

struct errx_status errx = { .type = errx_type_none };

extern void errx_type_none(void *p, char *out, size_t len) {
    strblit(out, len, "No error");
}

#define SEL4_ERR_PREFIX "seL4: "

static const char *error_strings_sel4[] = {
        SEL4_ERR_PREFIX "NoError",
        SEL4_ERR_PREFIX "InvalidArgument",
        SEL4_ERR_PREFIX "InvalidCapability",
        SEL4_ERR_PREFIX "IllegalOperation",
        SEL4_ERR_PREFIX "RangeError",
        SEL4_ERR_PREFIX "AlignmentError",
        SEL4_ERR_PREFIX "FailedLookup",
        SEL4_ERR_PREFIX "TruncatedMessage",
        SEL4_ERR_PREFIX "DeleteFirst",
        SEL4_ERR_PREFIX "RevokeFirst",
        SEL4_ERR_PREFIX "NotEnoughMemory"
};
#define ERR_COUNT 11
seL4_CompileTimeAssert(ERR_COUNT == sizeof(error_strings_sel4) / sizeof(*error_strings_sel4));

seL4_CompileTimeAssert(seL4_NoError == 0);
seL4_CompileTimeAssert(seL4_InvalidArgument == 1);
seL4_CompileTimeAssert(seL4_InvalidCapability == 2);
seL4_CompileTimeAssert(seL4_IllegalOperation == 3);
seL4_CompileTimeAssert(seL4_RangeError == 4);
seL4_CompileTimeAssert(seL4_AlignmentError == 5);
seL4_CompileTimeAssert(seL4_FailedLookup == 6);
seL4_CompileTimeAssert(seL4_TruncatedMessage == 7);
seL4_CompileTimeAssert(seL4_DeleteFirst == 8);
seL4_CompileTimeAssert(seL4_RevokeFirst == 9);
seL4_CompileTimeAssert(seL4_NotEnoughMemory == 10);
seL4_CompileTimeAssert(ERR_COUNT == 11);

extern void errx_type_sel4(void *p, char *out, size_t len) {
    struct errx_status *errx_status = (struct errx_status *) p;
    if (errx_status->extra >= ERR_COUNT) {
        out = strblitadv(out, &len, SEL4_ERR_PREFIX "Unknown Error");
    } else {
        out = strblitadv(out, &len, error_strings_sel4[errx_status->extra]);
    }
    errx_concat_traceback(errx_status, out, len);
}

static const char *error_strings_generic[] = _ERRX_GENERIC_STRINGS;

extern void errx_type_generic(void *p, char *out, size_t len) {
    struct errx_status *errx_status = (struct errx_status *) p;
    if (errx_status->extra >= sizeof(error_strings_generic) / sizeof(*error_strings_generic)) {
        out = strblitadv(out, &len, "Unknown Generic Error");
    } else {
        out = strblitadv(out, &len, error_strings_generic[errx_status->extra]);
    }
    errx_concat_traceback(errx_status, out, len);
}

extern void errx_concat_traceback(struct errx_status *errx_status, char *out, size_t len) {
    if (errx_status->traceback_next > 0) {
        out = strblitadv(out, &len, "\nTraceback (most recent call last):\n\t");
        for (int i = 0; i < errx_status->traceback_next; i++) {
            out = strblitadv(out, &len, errx_status->traceback_elems[i]);
            out = strblitadv(out, &len, "\n\t");
        }
        strblit(out, len, "-- display --");
    }
}
