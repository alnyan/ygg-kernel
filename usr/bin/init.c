#include <stdio.h>
#include <unistd.h>

void _start(void *arg) {
    char buf[5];

    int fd = open("/bin/test.txt", 0, O_RDONLY);

    printf("fd is %d\n", fd);

    if (fd != -1) {
        ssize_t bread;
        printf("--- File data ---\n");
        while ((bread = read(fd, buf, sizeof(buf))) > 0) {
            write(0, buf, bread);
        }
        printf("\n--- End data ---\n");
    }

    close(fd);

    while (1) {
        if (read(0, buf, 5) != 5) {
            exit(1234);
        }

        write(0, "Test: ", 6);
        write(0, buf, 5);
        write(0, "\n", 1);
    }
    exit(0);
}
