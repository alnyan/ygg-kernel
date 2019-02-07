#include "debug.h"

#if defined(ARCH_AARCH64)
#include "arch/aarch64/uart.h"
#endif

static const char *s_debug_xs_set0 = "0123456789abcdef";
static const char *s_debug_xs_set1 = "0123456789ABCDEF";

static void debugc(char c) {
#ifdef ARCH_AARCH64
    uart_send(0, c);
#endif
}

static void debugs(const char *s) {
    char c;
    while ((c = *(s++))) {
        debugc(c);
    }
}

static void debug_ds(int64_t x, char *res, int s, int sz) {
    if (!x) {
        res[0] = '0';
        res[1] = 0;
        return;
    }

    int c;
    uint64_t v;

    if (sz) {
        if (s && x < 0) {
            v = (uint64_t) -x;
        } else {
            s = 0;
            v = (uint64_t) x;
        }
    } else {
        if (s && ((int32_t) x) < 0) {
            v = (uint64_t) -((int32_t) x);
        } else {
            s = 0;
            v = (uint64_t) x;
        }
    }

    c = 0;

    while (v) {
        res[c++] = '0' + v % 10;
        v /= 10;
    }

    if (s) {
        res[c++] = '-';
    }

    res[c] = 0;

    for (int i = 0, j = c - 1; i < j; ++i, --j) {
        res[i] ^= res[j];
        res[j] ^= res[i];
        res[i] ^= res[j];
    }
}

static void debug_xs(uint64_t v, char *res, const char *set) {
    if (!v) {
        res[0] = '0';
        res[1] = 0;
        return;
    }

    int c = 0, x;

    while (v) {
        res[c++] = set[v & 0xF];
        v >>= 4;
    }

    res[c] = 0;

    for (int i = 0, j = c - 1; i < j; ++i, --j) {
        res[i] ^= res[j];
        res[j] ^= res[i];
        res[i] ^= res[j];
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
        char v_char;
        int32_t v_int32;
        uint32_t v_uint32;
        int64_t v_int64;
        uint64_t v_uint64;
    } value;
    char buf[64];

    while ((c = *fmt)) {
        switch (c) {
            case '%':
                c = *(++fmt);
                switch (c) {
                    case 'l':
                        c = *(++fmt);
                        switch (c) {
                            case 'd':
                                value.v_int64 = va_arg(args, int64_t);
                                debug_ds(value.v_int64, buf, 1, 1);
                                debugs(buf);
                                break;
                            case 'u':
                                value.v_uint64 = va_arg(args, uint64_t);
                                debug_ds(value.v_uint64, buf, 0, 1);
                                debugs(buf);
                                break;
                            case 'x':
                                value.v_uint64 = va_arg(args, uint64_t);
                                debug_xs(value.v_uint64, buf, s_debug_xs_set0);
                                debugs(buf);
                                break;
                            case 'X':
                                value.v_uint64 = va_arg(args, uint64_t);
                                debug_xs(value.v_uint64, buf, s_debug_xs_set1);
                                debugs(buf);
                                break;
                            default:
                                debugc('%');
                                debugc('l');
                                debugc(c);
                                break;
                        }
                        break;
                    case 'c':
                        // char is promoted to int
                        value.v_char = va_arg(args, int);
                        debugc(value.v_char);
                        break;
                    case 'd':
                        value.v_int64 = va_arg(args, int32_t);
                        debug_ds(value.v_int64 & 0xFFFFFFFF, buf, 1, 0);
                        debugs(buf);
                        break;
                    case 'u':
                        value.v_uint64 = va_arg(args, uint32_t);
                        debug_ds(value.v_uint64 & 0xFFFFFFFF, buf, 0, 0);
                        debugs(buf);
                        break;
                    case 'x':
                        value.v_uint64 = va_arg(args, uint32_t);
                        debug_xs(value.v_uint64 & 0xFFFFFFFF, buf, s_debug_xs_set0);
                        debugs(buf);
                        break;
                    case 'X':
                        value.v_uint64 = va_arg(args, uint32_t);
                        debug_xs(value.v_uint64 & 0xFFFFFFFF, buf, s_debug_xs_set1);
                        debugs(buf);
                        break;
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
