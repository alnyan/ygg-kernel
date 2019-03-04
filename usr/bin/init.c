#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

int readline(char *buf, size_t len) {
    size_t p = 0;
    char c;
    while (p < len - 1) {
        if (read(STDIN_FILENO, &c, 1) == 1) {
            if (c >= ' ') {
                buf[p++] = c;
                putc(c);
            } else if (c == '\b') {
                if (p) {
                    buf[p--] = 0;
                    putc('\b');
                }
                continue;
            } else if (c == '\n') {
                putc('\n');
                break;
            }
        } else {
            break;
        }
    }
    buf[p] = 0;
    return 0;
}

int execute(const char *cmd, const char *arg) {
    pid_t pid = fork();

    switch (pid) {
    case 0:
        execve(cmd, NULL, NULL);
        exit(1);
    case -1:
        printf("Exec error\n");
        return -1;
    default:
        break;
    }

    return 0;
}

void _start(void) {
    char input[256];
    char cmd[64];
    const char *arg = NULL;

    while (1) {
        printf("> ");
        readline(input, sizeof(input));

        // Split cmd and args
        arg = strchr(input, ' ');
        if (arg) {
            assert(arg - input < 64);

            strncpy(cmd, input, arg - input);
            cmd[arg - input] = 0;
            ++arg;
        } else {
            assert(strlen(input) < 64);

            strcpy(cmd, input);
        }

        printf("cmd: %s, arg: %s\n", cmd, arg);

        execute(cmd, arg);
    }

    exit(0);
}
