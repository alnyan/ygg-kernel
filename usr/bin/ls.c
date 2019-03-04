#include <stdio.h>
#include <unistd.h>

static char dt_type(int t) {
    switch (t) {
    case DT_REG:
        return 'f';
    case DT_DIR:
        return 'd';
    default:
        return '?';
    }
}

void _start(void) {
    struct dirent ent;
    int d = opendir("/");

    while (readdir(d, &ent) == 0) {
        printf(" %c %s\n", dt_type(ent.d_type), ent.d_name);
    }

    closedir(d);

    exit(0);
}
