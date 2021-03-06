#include "elf.h"
#include "linux/elf.h"
#include "sys/string.h"
#include "sys/task.h"
#include "sys/panic.h"
#include "sys/assert.h"
#include "sys/debug.h"
#include "sys/mem.h"
#include "sys/mm.h"
#include <stddef.h>

// FIXME
#ifndef ARCH_X86
#error "Not yet implemented"
#endif

// Loads ELF file into memory space `dst'
int elf_load(mm_pagedir_t dst, uintptr_t src_addr, size_t src_len) {
    kdebug("Trying to load ELF from %p, %uB\n", src_addr, src_len);

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *) src_addr;
    Elf32_Shdr *shdrs = (Elf32_Shdr *) (src_addr + ehdr->e_shoff);
    Elf32_Shdr *shstrtabh = &shdrs[ehdr->e_shstrndx];
    const char *shstrtabd = (const char *) (src_addr + shstrtabh->sh_offset);
    uint32_t entry_addr = ehdr->e_entry;

    // Validate ident
    if (strncmp((const char *) ehdr->e_ident, "\x7F" "ELF", 4)) {
        kerror("\tError: identity doesn't match ELF ident\n");
        return -1;
    }

    for (size_t i = 0; i < ehdr->e_shnum; ++i) {
        Elf32_Shdr *shdr = &shdrs[i];
        const char *name = NULL;

        if (shdr->sh_name) {
            name = &shstrtabd[shdr->sh_name];
        }

        // Load only program data
        if (shdr->sh_type == SHT_PROGBITS || shdr->sh_type == SHT_NOBITS) {
            if (shdr->sh_flags & SHF_ALLOC) {
                // Just alloc pages for the section
                uintptr_t page_start = shdr->sh_addr & -0x1000;
                uintptr_t page_end = MM_ALIGN_UP(shdr->sh_addr + shdr->sh_size, 0x1000);
                size_t page_count = (page_end - page_start) / 0x1000;

                assert(mm_map_range(dst, page_start, page_count, MM_FLG_WR | MM_FLG_US | MM_FLG_MERGE) == 0);

                if (shdr->sh_type == SHT_PROGBITS) {
                    kdebug("Loading section %s\n", name);
                    assert(mm_memcpy_kernel_to_user(dst, (void *) shdr->sh_addr, (const void *) (shdr->sh_offset + (uintptr_t) ehdr), shdr->sh_size) == 0);
                }
            }
        }
    }

    kdebug("DONE! entry is %p, welcome\n", entry_addr);

    return (int) entry_addr;
}
