#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <elf.h>

struct symbol {
    uint32_t addr;
    uint32_t type;
    const char *name;
};

int cmpsym(const void *a, const void *b) {
    const struct symbol *sa = a;
    const struct symbol *sb = b;

    if (sa->addr > sb->addr) {
        return 1;
    }

    if (sa->addr == sb->addr) {
        return 0;
    }

    return -1;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        return -1;
    }

    struct stat st;

    if (stat(argv[1], &st) != 0) {
        perror("stat()");
        return -1;
    }

    char *data = malloc(st.st_size);
    size_t size = 0;
    ssize_t bread;
    FILE *fp = fopen(argv[1], "rb");

    while ((bread = fread(data + size, 1, st.st_size - size >= 4096 ? 4096 : st.st_size - size, fp))) {
        size += bread;
    }

    fclose(fp);

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *) data;
    Elf32_Shdr *shdrs = (Elf32_Shdr *) (data + ehdr->e_shoff);
    Elf32_Shdr *strtabh = &shdrs[ehdr->e_shstrndx];
    Elf32_Shdr *symtabh = NULL;
    const char *strtabd = data + strtabh->sh_offset;
    const char *strtab2 = NULL;
    uint32_t nsyms;

    // Check signature
    if (strncmp("\x7F" "ELF", (const char *) ehdr, 4)) {
        printf("Invalid magic\n");
        return -1;
    }

    for (int i = 0; i < ehdr->e_shnum; ++i) {
        Elf32_Shdr *shdr = &shdrs[i];

        const char *section_name = &strtabd[shdr->sh_name];

        if (!strcmp(section_name, ".symtab")) {
            symtabh = shdr;
        }

        if (!strcmp(section_name, ".strtab")) {
            strtab2 = shdr->sh_offset + data;
        }

        printf(" [%d] %s\n", i, section_name);
    }

    if (!symtabh || !strtab2) {
        printf("Failed to find .symtab\n");
        return -1;
    }

    Elf32_Sym *symbols = (Elf32_Sym *) (data + symtabh->sh_offset);
    nsyms = symtabh->sh_size / sizeof(Elf32_Sym);

    struct symbol *syms = malloc(sizeof(struct symbol) * nsyms);
    int symc = 0;

    for (int i = 0; i < nsyms; ++i) {
        Elf32_Sym *sym = &symbols[i];
        int type = ELF32_ST_TYPE(sym->st_info);
        if (sym->st_name && (type == STT_FUNC || type == STT_OBJECT)) {
            const char *name = &strtab2[sym->st_name];

            syms[symc].name = name;
            syms[symc].type = type;
            syms[symc++].addr = sym->st_value;
        }
    }

    qsort((const void *) syms, symc, sizeof(struct symbol), cmpsym);

    for (int i = 0; i < symc; ++i) {
        printf("%p %c %s\n", syms[i].addr, syms[i].type == STT_FUNC ? 'F' : 'O', syms[i].name);
    }

    free(syms);
    free(data);

    return 0;
}
