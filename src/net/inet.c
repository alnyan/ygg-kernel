#include "inet.h"
#include "sys/debug.h"
#include "sys/string.h"
#include "sys/assert.h"
#include "sys/atoi.h"

void inet_ntoa(char *out, inaddr_t a) {
    size_t l = 0;
    for (int i = 0; i < 4; ++i) {
        debug_ds((a >> (i * 8)) & 0xFF, &out[l], 0, 0);
        l = strlen(out);
        if (i != 3) {
            out[l++] = '.';
            out[l] = 0;
        }
    }
}

inaddr_t inet_aton(const char *in) {
    uint32_t r = 0;
    const char *p = in;

    for (int i = 0; i < 4; ++i) {
        uint8_t octet = (uint8_t) atoi(p);
        if (i != 3) {
            assert(p = strchr(p, '.'));
            ++p;
        }
        r |= (octet) << (8 * i);
    }
    return r;
}
