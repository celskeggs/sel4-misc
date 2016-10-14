#include <bedrock/debug.h>
#include <init/init.h>
#include <ipc/ipc.h>
#include <resource/mem_page.h>

bool main(void) {
    debug_println("hello, sandbox world!");
    struct mem_page_cookie cookie = {};
    if (!mem_page_map((void *) 0xFFFF0000, &cookie)) {
        ERRX_TRACEPOINT;
        return false;
    }
    return true;
}
