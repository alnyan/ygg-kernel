#include "stdio.h"
#include "unistd.h"
#include "string.h"
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

static const char *s_print_xs_set0 = "0123456789abcdef";
static const char *s_print_xs_set1 = "0123456789ABCDEF";

static void printspl(const char *s, char p, size_t c) {
    size_t l = strlen(s);
    for (size_t i = l; i < c; ++i) {
        putc(p);
    }
    puts(s);
}

static void printspr(const char *s, char p, size_t c) {
    size_t l = strlen(s);
    puts(s);
    for (size_t i = l; l < c; ++i) {
        putc(p);
    }
}

static void print_ds(int64_t x, char *res, int s, int sz) {
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

void print_xs(uint64_t v, char *res, const char *set) {
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

void printfv(const char *fmt, va_list args) {
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
                                print_ds(value.v_int64, buf, 1, 1);
                                puts(buf);
                                break;
                            case 'u':
                                value.v_uint64 = va_arg(args, uint64_t);
                                print_ds(value.v_uint64, buf, 0, 1);
                                puts(buf);
                                break;
                            case 'x':
                                value.v_uint64 = va_arg(args, uint64_t);
                                print_xs(value.v_uint64, buf, s_print_xs_set0);
                                puts(buf);
                                break;
                            case 'X':
                                value.v_uint64 = va_arg(args, uint64_t);
                                print_xs(value.v_uint64, buf, s_print_xs_set1);
                                puts(buf);
                                break;
                            default:
                                putc('%');
                                putc('l');
                                putc(c);
                                break;
                        }
                        break;
                    case 'c':
                        // char is promoted to int
                        value.v_char = va_arg(args, int);
                        putc(value.v_char);
                        break;
                    case 'd':
                        value.v_int64 = va_arg(args, int32_t);
                        print_ds(value.v_int64 & 0xFFFFFFFF, buf, 1, 0);
                        puts(buf);
                        break;
                    case 'u':
                        value.v_uint64 = va_arg(args, uint32_t);
                        print_ds(value.v_uint64 & 0xFFFFFFFF, buf, 0, 0);
                        puts(buf);
                        break;
                    case 'x':
                        value.v_uint64 = va_arg(args, uint32_t);
                        print_xs(value.v_uint64 & 0xFFFFFFFF, buf, s_print_xs_set0);
                        puts(buf);
                        break;
                    case 'X':
                        value.v_uint64 = va_arg(args, uint32_t);
                        print_xs(value.v_uint64 & 0xFFFFFFFF, buf, s_print_xs_set1);
                        puts(buf);
                        break;
                    case 'p':
                        value.v_ptr = va_arg(args, uintptr_t);
                        putc('0');
                        putc('x');
                        print_xs(value.v_ptr, buf, s_print_xs_set0);
                        printspl(buf, '0', sizeof(uintptr_t) * 2);
                        break;
                    case 's':
                        value.v_string = va_arg(args, const char *);
                        puts(value.v_string ? value.v_string : "(null)");
                        break;
                    default:
                        putc('%');
                        putc(c);
                        break;
                }
                break;
            default:
                putc(c);
                break;
        }

        ++fmt;
    }
}

void printf(const char *f, ...) {
    va_list args;
    va_start(args, f);
    printfv(f, args);
    va_end(args);
}
