#include "atoi.h"

int atoi(const char *s) {
    // TODO: negative numbers
    int r = 0;
    char c;
    while ((c = *s++)) {
        if (c >= '0' && c <= '9') {
            r *= 10;
            r += c - '0';
        } else {
            break;
        }
    }
    return r;
}
