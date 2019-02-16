#include <stddef.h>
#include "sys/debug.h"
#include "sys/string.h"

void *memset(void *block, int v, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ((char *) block)[i] = v;
    }
    return block;
}

void *memcpy(void *dst, const void *src, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ((char *) dst)[i] = ((const char *) src)[i];
    }
    return dst;
}

static char *s_fmtsiz_pwrs = "kMGPTE???";

void fmtsiz(size_t sz, char *out) {
    size_t f = sz, r = 0;
    int pwr = 0;
    size_t l = 0;

    while (f >= 1536) {
        r = ((f % 1024) * 10) / 1024;
        f /= 1024;
        ++pwr;
    }

    /* TODO: make debug_ds return number of chars produced */
    debug_ds(f, out, 0, 0);
    l = strlen(out);

    if (pwr) {
        out[l++] = '.';
        out[l++] = '0' + r;

        out[l++] = ' ';
        out[l++] = s_fmtsiz_pwrs[pwr - 1];
    }

    out[l++] = 'B';
    out[l++] = 0;
}
