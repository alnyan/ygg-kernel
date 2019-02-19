#include <stdio.h>
#include <string.h>
#include <unistd.h>

void _start(void *arg) {
    char keybuf;
    char line[256];
    size_t pos = 0;

    while (1) {
        printf("> ");

        while (1) {
            if (read(0, &keybuf, 1) != 1) {
                exit(-1);
            }

            if (keybuf >= ' ') {
                line[pos++] = keybuf;
                write(0, &keybuf, 1);
            } else if (keybuf == '\b') {
                if (pos) {
                    line[pos--] = 0;
                    write(0, &keybuf, 1);
                }
            } else if (keybuf == '\n') {
                write(0, &keybuf, 1);
                line[pos] = 0;
                pos = 0;
                break;
            }
        }

        printf("You've typed: \"%s\"\n", line);

        if (!strcmp(line, "crashme") || !strcmp(line, "exit")) {
            exit(1234);
        }
    }

    exit(-1);
}
