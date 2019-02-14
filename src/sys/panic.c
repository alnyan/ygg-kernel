#include "panic.h"
#include "debug.h"

void panicf(const char *fmt, ...) {
    debug(PANIC_MSG_INTRO);
    va_list args;
    va_start(args, fmt);
    debugfv(fmt, args);
    va_end(args);

    debug("--- (No dumps provided) ---\n");

    panic_hlt();
}
