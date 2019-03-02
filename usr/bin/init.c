#include <stdio.h>
#include <string.h>
#include <unistd.h>

void _start(void *arg) {
    // Do something for testing
    write(STDOUT_FILENO, "Hello\n", 6);

    while (1) {}

    // char keybuf;
    // char line[256];
    // size_t pos = 0;

    // while (1) {
    //     printf("> ");

    //     while (1) {
    //         if (read(STDIN_FILENO, &keybuf, 1) != 1) {
    //             exit(-1);
    //         }

    //         if (keybuf >= ' ') {
    //             line[pos++] = keybuf;
    //             write(STDOUT_FILENO, &keybuf, 1);
    //         } else if (keybuf == '\b') {
    //             if (pos) {
    //                 line[pos--] = 0;
    //                 write(STDOUT_FILENO, &keybuf, 1);
    //             }
    //         } else if (keybuf == '\n') {
    //             write(STDOUT_FILENO, &keybuf, 1);
    //             line[pos] = 0;
    //             pos = 0;
    //             break;
    //         }
    //     }

    //     printf("You've typed: \"%s\"\n", line);

    //     if (!strcmp(line, "crashme") || !strcmp(line, "exit")) {
    //         exit(1234);
    //     }

    //     if (!strcmp(line, "fello")) {
    //         fexecve("/bin/hello", NULL, NULL);
    //     }

    //     if (!strcmp(line, "hello")) {
    //         // fexecve("/bin/hello", NULL, NULL);
    //         switch (fork()) {
    //         case 0:
    //             write(STDOUT_FILENO, "Hello!\n", 6);
    //             execve("/bin/hello", NULL, NULL);
    //             exit(1);
    //         case -1:
    //             printf("fork() failed\n");
    //             exit(4321);
    //         default:
    //             printf("fork() succeeded\n");
    //             break;
    //         }
    //     } else if (!strcmp(line, "ls")) {
    //         int f = opendir("/proc");
    //         if (f == -1) {
    //             printf("opendir failed\n");
    //         } else {
    //             struct dirent ent;

    //             while (readdir(f, &ent) == 0) {
    //                 printf(" %s %c\n", ent.d_name, ent.d_type == DT_REG ? 'R' : 'D');
    //             }

    //             closedir(f);
    //         }
    //     } else if (!strcmp(line, "test")) {
    //         int f = open("/proc/test", 0, O_RDONLY);

    //         if (f == -1) {
    //             printf("open failed\n");
    //         } else {
    //             printf("open succeeded\n");
    //             int v;
    //             printf("read returned %d\n", read(f, &v, sizeof(int)));
    //             printf(" = %d\n", v);
    //             close(f);
    //         }
    //     }
    // }

    exit(-1);
}
