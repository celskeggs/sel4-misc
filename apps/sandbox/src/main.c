#include <bedrock/debug.h>
#include <init/init.h>
#include <ipc/ipc.h>
#include <resource/mem_page.h>
#include <elfloader/elfexec.h>

extern char *image_registrar;
extern char *image_registrar_end;

bool main(void) {
    debug_println("hello, sandbox world!");
    struct elfexec context;
    if (!elfexec_init(image_registrar, image_registrar_end - image_registrar, &context, seL4_CapNull, 255, seL4_CapNull)) {
        ERRX_TRACEPOINT;
        return false;
    }
    if (!elfexec_start(&context)) {
        ERRX_TRACEPOINT;
        return false;
    }
    return true;
}
