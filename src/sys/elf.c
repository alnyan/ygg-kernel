#include "elf.h"
#include "sys/string.h"
#include "sys/debug.h"
#include "sys/mem.h"
#include "sys/mm.h"
#include <stddef.h>

// Loads ELF file into memory space `dst'
int elf_load(mm_pagedir_t dst, uintptr_t src_addr, size_t src_len) {
    debug("Trying to load ELF from %p, %uB\n", src_addr, src_len);

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *) src_addr;

    // Validate ident
    if (strncmp(ehdr->e_ident, "\x7F" "ELF", 4)) {
        debug("\tError: identity doesn't match ELF ident\n");
        return -1;
    }

    return -1;
}
