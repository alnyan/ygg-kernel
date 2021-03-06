#include "debug.h"
#include "string.h"
#include <stdint.h>
#include "sys/ctype.h"

#if defined(ARCH_AARCH64)
#include "arch/aarch64/uart.h"
#endif
#if defined(ARCH_X86)
#include "arch/x86/hw/com.h"
#include "arch/x86/hw/console.h"
#endif

static const char *s_debug_xs_set0 = "0123456789abcdef";
static const char *s_debug_xs_set1 = "0123456789ABCDEF";

static int s_debug_serial_level = DEBUG_DEFAULT;
static int s_debug_disp_level = DEBUG_INFO;

void debugc(int level, char c) {
#ifdef ARCH_AARCH64
    uart_send(0, c);
#endif
#ifdef ARCH_X86
    if (level >= s_debug_serial_level) {
	    com_send(X86_COM0, c);
    }
    if (level >= s_debug_disp_level) {
        x86_con_putc(c);
    }
#endif
}

void debugs(int level, const char *s) {
    char c;
    while ((c = *(s++))) {
        debugc(level, c);
    }
}

static void debugspl(int level, const char *s, char p, size_t c) {
    size_t l = strlen(s);
    for (size_t i = l; i < c; ++i) {
        debugc(level, p);
    }
    debugs(level, s);
}

static void debugspr(int level, const char *s, char p, size_t c) {
    size_t l = strlen(s);
    debugs(level, s);
    for (size_t i = l; i < c; ++i) {
        debugc(level, p);
    }
}

static void debugsp(int level, const char *s, char padc, int padl) {
    if (padl > 0) {
        debugspl(level, s, padc, padl);
    } else {
        debugspr(level, s, padc, -padl);
    }
}

void debug_ds(int64_t x, char *res, int s, int sz) {
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

void debug_xs(uint64_t v, char *res, const char *set) {
    if (!v) {
        res[0] = '0';
        res[1] = 0;
        return;
    }

    int c = 0;

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

void debugf(int level, const char *f, ...) {
    va_list args;
    va_start(args, f);
    debugfv(level, f, args);
    va_end(args);
}

void debugfv(int level, const char *fmt, va_list args) {
    char c;
    union {
        const char *v_string;
        char v_char;
        int32_t v_int32;
        uint32_t v_uint32;
        int64_t v_int64;
        uint64_t v_uint64;
        uintptr_t v_ptr;
    } value;
    char buf[64];
    char padc;
    int padn;
    int padd;

    while ((c = *fmt)) {
        switch (c) {
            case '%':
                c = *(++fmt);

                padc = ' ';
                padd = 1;
                padn = 0;
                if (c == '0') {
                    padc = c;
                }

                if (c == '-') {
                    padd = -1;
                    c = *(++fmt);
                }

                while (c >= '0' && c <= '9') {
                    padn *= 10;
                    padn += padd * (int) (c - '0');
                    c = *(++fmt);
                }

                switch (c) {
                    case 'l':
                        c = *(++fmt);
                        switch (c) {
                            case 'd':
                                value.v_int64 = va_arg(args, int64_t);
                                debug_ds(value.v_int64, buf, 1, 1);
                                debugsp(level, buf, padc, padn);
                                break;
                            case 'u':
                                value.v_uint64 = va_arg(args, uint64_t);
                                debug_ds(value.v_uint64, buf, 0, 1);
                                debugsp(level, buf, padc, padn);
                                break;
                            case 'x':
                                value.v_uint64 = va_arg(args, uint64_t);
                                debug_xs(value.v_uint64, buf, s_debug_xs_set0);
                                debugsp(level, buf, padc, padn);
                                break;
                            case 'X':
                                value.v_uint64 = va_arg(args, uint64_t);
                                debug_xs(value.v_uint64, buf, s_debug_xs_set1);
                                debugsp(level, buf, padc, padn);
                                break;
                            case 'p':
                                value.v_uint64 = va_arg(args, uint64_t);
                                debugc(level, '0');
                                debugc(level, 'x');
                                debug_xs(value.v_uint64, buf, s_debug_xs_set0);
                                debugspl(level, buf, '0', sizeof(uint64_t) * 2);
                                break;
                            default:
                                debugc(level, '%');
                                debugc(level, 'l');
                                debugc(level, c);
                                break;
                        }
                        break;
                    case 'c':
                        // char is promoted to int
                        value.v_char = va_arg(args, int);
                        debugc(level, value.v_char);
                        break;
                    case 'd':
                        value.v_int64 = va_arg(args, int32_t);
                        debug_ds(value.v_int64 & 0xFFFFFFFF, buf, 1, 0);
                        debugsp(level, buf, padc, padn);
                        break;
                    case 'u':
                        value.v_uint64 = va_arg(args, uint32_t);
                        debug_ds(value.v_uint64 & 0xFFFFFFFF, buf, 0, 0);
                        debugsp(level, buf, padc, padn);
                        break;
                    case 'x':
                        value.v_uint64 = va_arg(args, uint32_t);
                        debug_xs(value.v_uint64 & 0xFFFFFFFF, buf, s_debug_xs_set0);
                        debugsp(level, buf, padc, padn);
                        break;
                    case 'X':
                        value.v_uint64 = va_arg(args, uint32_t);
                        debug_xs(value.v_uint64 & 0xFFFFFFFF, buf, s_debug_xs_set1);
                        debugsp(level, buf, padc, padn);
                        break;
                    case 'p':
                        value.v_ptr = va_arg(args, uintptr_t);
                        debugc(level, '0');
                        debugc(level, 'x');
                        debug_xs(value.v_ptr, buf, s_debug_xs_set0);
                        debugspl(level, buf, '0', sizeof(uintptr_t) * 2);
                        break;
                    case 's':
                        value.v_string = va_arg(args, const char *);
                        debugsp(level, value.v_string ? value.v_string : "(null)", padc, padn);
                        break;
                    default:
                        debugc(level, '%');
                        debugc(level, c);
                        break;
                }
                break;
            default:
                debugc(level, c);
                break;
        }

        ++fmt;
    }
}

#define DEBUG_DUMP_LINE     16
// TODO: move this to some kind of config
#define DEBUG_DUMP_WORDS

void debug_dump(int level, const void *block, size_t len) {
    debugf(level, "--- Memory dump at %p, %uB ---\n", block, len);

    for (size_t i = 0; i < len; i += DEBUG_DUMP_LINE) {
        debugf(level, "%p: ", (uintptr_t) block + i);

#if defined(DEBUG_DUMP_WORDS)
        for (size_t j = i; j < i + DEBUG_DUMP_LINE; j += 2) {
            if (j < len) {
#if defined(ARCH_X86)
                debugc(level, s_debug_xs_set1[((((const uint16_t *) block)[j / 2] >> 4) & 0xF)]);
                debugc(level, s_debug_xs_set1[((((const uint16_t *) block)[j / 2]) & 0xF)]);
                debugc(level, s_debug_xs_set1[((((const uint16_t *) block)[j / 2] >> 12) & 0xF)]);
                debugc(level, s_debug_xs_set1[((((const uint16_t *) block)[j / 2] >> 8) & 0xF)]);
#else
                debugc(level, s_debug_xs_set1[((((const uint16_t *) block)[j / 2] >> 12) & 0xF)]);
                debugc(level, s_debug_xs_set1[((((const uint16_t *) block)[j / 2] >> 8) & 0xF)]);
                debugc(level, s_debug_xs_set1[((((const uint16_t *) block)[j / 2] >> 4) & 0xF)]);
                debugc(level, s_debug_xs_set1[((((const uint16_t *) block)[j / 2]) & 0xF)]);
#endif
            } else {
                debugc(level, ' ');
                debugc(level, ' ');
            }
            debugc(level, ' ');
        }
#else
        for (size_t j = i; j < i + DEBUG_DUMP_LINE; ++j) {
            if (j < len) {
                debugc(level, s_debug_xs_set1[((((const uint8_t *) block)[j] >> 4) & 0xF)]);
                debugc(level, s_debug_xs_set1[((((const uint8_t *) block)[j]) & 0xF)]);
            } else {
                debugc(level, ' ');
                debugc(level, ' ');
            }
            debugc(level, ' ');
        }
#endif
        debugc(level, ' ');

        for (size_t j = i; j < i + DEBUG_DUMP_LINE && j < len; ++j) {
            char c = ((const char *) block)[j];
            if (isprint(c)) {
                debugc(level, c);
            } else {
                debugc(level, '.');
            }
        }

        debugc(level, '\n');
    }

    debugf(level, "--- End dump ---\n");
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
