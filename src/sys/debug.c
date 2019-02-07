#include "debug.h"
#include "arch/aarch64/uart.h"

static void debugc(char c) {
    uart_send(0, c);
}

void debugf(const char *f, ...) {
    va_list args;
    va_start(args, f);
    debugfv(f, args);
    va_end(args);
}

void debugfv(const char *fmt, va_list args) {
    char c;
    while ((c = *fmt)) {
        switch (c) {
            case '%':
                c = *(++fmt);
                switch (c) {
                    default:
                        debugc('%');
                        debugc(c);
                        break;
                }
                break;
            default:
                debugc(c);
                break;
        }

        ++fmt;
    }
}
