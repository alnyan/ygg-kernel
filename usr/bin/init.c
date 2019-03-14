#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

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
        perror("opendir()");
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
        perror("open()");
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

static int b_hex(const char *arg) {
    static const int b_hex_len = 16;
    int f = open(arg, 0, O_RDONLY);
    if (f == -1) {
        perror("open()");
        return -1;
    }
    char buf[256];
    ssize_t rd;

    while ((rd = read(f, buf, sizeof(buf))) > 0) {
        for (int i = 0; i < rd; i += b_hex_len) {
            for (int j = 0; j < b_hex_len; ++j) {
                if (i + j < rd) {
                    printf("%02x ", buf[i + j] & 0xFF);
                } else {
                    printf("   ");
                }
            }

            printf("| ");
            for (int j = 0; j < b_hex_len && i + j < rd; ++j) {
                char c = buf[i + j];
                printf("%c", c >= ' ' ? c : '.');
            }
            printf("\n");
        }
    }

    close(f);
    return 0;
}

static int b_pid(const char *arg) {
    printf("My PID is %d\n", getpid());
    return 0;
}

static int b_exit(const char *arg) {
    exit(0);
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
    },
    {
        "hex",
        b_hex
    },
    {
        "pid",
        b_pid
    },
    {
        "exit",
        b_exit
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
        printf("Sorry, I've disabled execve() for now\n");
        //if (execve(cmd, NULL, NULL) != 0) {
        //    perror("execve()");
        //}
        exit(1);
    case -1:
        perror("fork()");
        return -1;
    default:
        waitpid(pid, NULL, 0);
        break;
    }

    return 0;
}

int main(void) {
    char input[256];
    char cmd[64];
    const char *arg = NULL;

    read(STDOUT_FILENO, input, 4);
    printf("Test 1234 %s\n", input);
    while (1);

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
