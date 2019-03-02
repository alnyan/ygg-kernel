#include "panic.h"
#include "debug.h"

void panicf(const char *fmt, ...) {
    kfatal(PANIC_MSG_INTRO);
    va_list args;
    va_start(args, fmt);
    debugfv(DEBUG_FATAL, fmt, args);
    va_end(args);

    panic_reg();

    panic_hlt();
}
