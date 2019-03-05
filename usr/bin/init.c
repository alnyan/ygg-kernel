#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>

typedef struct {
    const char *cmd;
    int (*func) (const char *);
} builtin_t;

static int b_echo(const char *arg) {
    printf("%s\n", arg);
    return 0;
}

static int b_ls(const char *arg) {
    int d = opendir(arg ? arg : "/");
    if (d == -1) {
        return -1;
    }
    struct dirent ent;
    while (readdir(d, &ent) == 0) {
        printf("%s\n", ent.d_name);
    }
    closedir(d);
    return 0;
}

static int b_cat(const char *arg) {
    int f = open(arg, 0, O_RDONLY);
    if (f == -1) {
        return -1;
    }
    char buf[256];
    ssize_t rd;
    while ((rd = read(f, buf, sizeof(buf))) > 0) {
        write(STDOUT_FILENO, buf, rd);
    }
    close(f);
    return 0;
}

static builtin_t builtins[] = {
    {
        "echo",
        b_echo
    },
    {
        "ls",
        b_ls
    },
    {
        "cat",
        b_cat
    }
};

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
    for (int i = 0; i < sizeof(builtins) / sizeof(builtins[0]); ++i) {
        if (!strcmp(builtins[i].cmd, cmd)) {
            return builtins[i].func(arg);
        }
    }

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
        waitpid(pid, NULL, 0);
        break;
    }

    return 0;
}

int main(void *argp) {
    char input[256];
    char cmd[64];
    const char *arg = NULL;

    while (1) {
        printf("# ");
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

        execute(cmd, arg);
    }

    exit(0);
}
