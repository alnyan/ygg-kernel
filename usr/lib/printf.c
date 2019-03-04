#include "stdio.h"
#include "unistd.h"
#include "string.h"
void printf(const char *fmt, ...) {
    char buf[1024];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    write(STDOUT_FILENO, buf, strlen(buf));
}
