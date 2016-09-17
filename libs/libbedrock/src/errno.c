#include <sel4/assert.h>
#include <bedrock/errno.h>

static const char *error_strings[] = {
    "NoError",
    "InvalidArgument",
    "InvalidCapability",
    "IllegalOperation",
    "RangeError",
    "AlignmentError",
    "FailedLookup",
    "TruncatedMessage",
    "DeleteFirst",
    "RevokeFirst",
    "NotEnoughMemory"
};
#define ERR_COUNT 11

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

const char *err_to_string(seL4_Error err) {
    if (err < 0 || err >= ERR_COUNT) {
        return "UnknownError";
    } else {
        return error_strings[err];
    }
}
