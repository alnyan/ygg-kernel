#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        return -1;
    }

    char *line = NULL;
    size_t line_sz = 0;
    ssize_t len;
    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        printf("Failed to open map file\n");
        return -1;
    }

    uint32_t addr = 0;

    sscanf(argv[2], "0x%x", &addr);

    char *prev_name = NULL;
    uint32_t prev_addr = 0;

    while ((len = getline(&line, &line_sz, fp)) > 0) {
        uint32_t saddr;
        sscanf(line, "0x%x", &saddr);

        if (addr < saddr) {
            if (prev_name) {
                for (int i = 0; i < strlen(prev_name); ++i) {
                    if (prev_name[i] == '\n') {
                        prev_name[i] = 0;
                        break;
                    }
                }

                printf("0x%x <%s + %u>\n", addr, prev_name, addr - prev_addr);
            }

            goto finish;
        }

        const char *e = strchr(line, ' ');
        if (!e) {
            printf("Broken file\n");
            return -1;
        }
        e = strchr(e + 1, ' ');
        if (!e) {
            printf("Broken file\n");
            return -1;
        }

        free(prev_name);
        prev_name = strdup(e + 1);
        prev_addr = saddr;
    }

    printf("(null)\n");

finish:
    free(line);
    free(prev_name);
    fclose(fp);


    return 0;
}
