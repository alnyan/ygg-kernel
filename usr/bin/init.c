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
            if (read(STDIN_FILENO, &keybuf, 1) != 1) {
                exit(-1);
            }

            if (keybuf >= ' ') {
                line[pos++] = keybuf;
                write(STDOUT_FILENO, &keybuf, 1);
            } else if (keybuf == '\b') {
                if (pos) {
                    line[pos--] = 0;
                    write(STDOUT_FILENO, &keybuf, 1);
                }
            } else if (keybuf == '\n') {
                write(STDOUT_FILENO, &keybuf, 1);
                line[pos] = 0;
                pos = 0;
                break;
            }
        }

        printf("You've typed: \"%s\"\n", line);

        if (!strcmp(line, "crashme") || !strcmp(line, "exit")) {
            exit(1234);
        }

        if (!strcmp(line, "hello")) {
            // fexecve("/bin/hello", NULL, NULL);
            switch (fork()) {
            case 0:
                write(STDOUT_FILENO, "Hello!\n", 6);
                execve("/bin/hello", NULL, NULL);
                exit(1);
            case -1:
                printf("fork() failed\n");
                exit(4321);
            default:
                printf("fork() succeeded\n");
                break;
            }
        }
    }

    exit(-1);
}
