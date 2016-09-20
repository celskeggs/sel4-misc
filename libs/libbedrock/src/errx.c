#include <sel4/assert.h>
#include <bedrock/errx.h>
#include <bedrock/buffer.h>

#define SEL4_ERR_PREFIX "seL4: "

static const char *error_strings[] = {
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
seL4_CompileTimeAssert(ERR_COUNT == sizeof(error_strings) / sizeof(*error_strings));

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

extern void errx_type_none(uint64_t extra, char *out, size_t len) {
    strblit(out, len, "No error");
}

extern void errx_type_sel4(uint64_t extra, char *out, size_t len) {
    if (extra >= ERR_COUNT) {
        strblit(out, len, SEL4_ERR_PREFIX "Unknown Error");
    } else {
        strblit(out, len, error_strings[extra]);
    }
}
