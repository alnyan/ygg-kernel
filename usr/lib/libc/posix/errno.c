#include "errno.h"
#include "stdio.h"

int errno;

const char *strerror(int e) {
    switch (e) {
    case 0:
        return "no error";
    case EPERM:
        return "operation not permitted";
    case ENOENT:
        return "no such file or directory";
    default:
        return "unknown error";
    }
}

void perror(const char *v) {
    printf("%s: %s\n", v, strerror(errno));
}
