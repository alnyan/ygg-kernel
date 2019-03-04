#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>

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
        if (execve(cmd, NULL, NULL) != 0) {
            printf("execve() error\n");
        }
        exit(1);
    case -1:
        printf("fork() error\n");
        return -1;
    default:
        break;
    }

    return 0;
}

void sigabrt_handler(int signum) {
    // Lol
    printf("Something happened, I'm SIGABRT handler\n");
    // Will work properly once I implement the sigreturn syscall
}

int main(void) {
    char input[256];
    char cmd[64];
    const char *arg = NULL;

    signal(SIGABRT, sigabrt_handler);

    printf("String: `%s'\n", "Test string");
    printf("int: %d\n", -123);
    printf("uint: %u\n", 0xFFFFFFFF);
    printf("ptr: %p\n", input);
    printf("hex: %x\n", 0xDEADBEEF);
    printf("lhex: %X\n", 0xDEADBEEF);
    printf("char: `%c'\n", '/');

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

        if (!strcmp(cmd, "segv")) {
            int *v = (int *) 0;
            *v = 3;
            continue;
        }

        if (!strcmp(cmd, "abrt")) {
            assert(0);
        }

        execute(cmd, arg);
    }

    exit(0);
}
