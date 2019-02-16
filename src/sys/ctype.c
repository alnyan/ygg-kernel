#include "ctype.h"

int isprint(char c) {
    return c >= 0x20 && c <= 0x7F;
}
