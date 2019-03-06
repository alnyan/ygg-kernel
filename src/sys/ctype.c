#include "ctype.h"

int isspace(char c) {
    return c == '\n' || c == '\t' || c == ' ';
}

int isprint(char c) {
    return c >= 0x20 && c <= 0x7F;
}

int islower(char c) {
    return 'a' <= c && c <= 'z';
}

int isupper(char c) {
    return 'A' <= c && c <= 'Z';
}

char tolower(char c) {
    return isupper(c) ? (c | 0x20) : c;
}

char toupper(char c) {
    return islower(c) ? (c & 0x5F) : c;
}

char togglecase(char c) {
    if (isupper(c)) {
        return c | 0x20;
    } else {
        return toupper(c);
    }
}
