#include "debug.h"

#if defined(ARCH_AARCH64)
#include "arch/aarch64/uart.h"
#endif

static void debugc(char c) {
#ifdef ARCH_AARCH64
    uart_send(0, c);
#endif
}

static void debugs(const char *s) {
    char c;
    while ((c = *s++)) {
        debugc(c);
    }
}

void debugf(const char *f, ...) {
    va_list args;
    va_start(args, f);
    debugfv(f, args);
    va_end(args);
}

void debugfv(const char *fmt, va_list args) {
    char c;
    union {
        const char *v_string;
    } value;

    while ((c = *fmt)) {
        switch (c) {
            case '%':
                c = *(++fmt);
                switch (c) {
                    case 's':
                        value.v_string = va_arg(args, const char *);
                        debugs(value.v_string ? value.v_string : "(null)");
                        break;
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

void debug_init(void) {
#if defined(ARCH_AARCH64)
    if (!uart_state(0)) {
        uart_default_config(0);
        debug("Set up UART debugging\n");
    } else {
        debug("UART is already initialized, skipping\n");
    }

    if (!uart_state(0)) {
        debug("Failed to setup UART for debugging\n");
    }
#endif
}
