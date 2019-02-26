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
                // TODO: This is platform-specific code and needs to be moved somewhere
                uintptr_t page_start = shdr->sh_addr & -MM_PAGESZ;
                uintptr_t page_end = MM_ALIGN_UP(shdr->sh_addr + shdr->sh_size, MM_PAGESZ);
                size_t page_count = (page_end - page_start) / MM_PAGESZ;

                if (page_count != 1) {
                    panic("NYI\n");
                }

                uintptr_t page_phys = mm_lookup(dst, page_start, MM_FLG_HUGE);

                if (page_phys == MM_NADDR) {
                    uintptr_t page;
                    assert((page = mm_alloc_phys_page()) != MM_NADDR);
                    mm_map_page(dst, page_start, page, MM_FLG_RW | MM_FLG_US);
                    page_phys = page;
                }

                // Map the page into kernel space
                // 0x400000 is the base for write access
                // TODO: maybe use copy_to_user
                mm_map_page(mm_kernel, 0x400000, page_phys, MM_FLG_RW);

                if (shdr->sh_type == SHT_PROGBITS) {
                    memcpy((void *) (shdr->sh_addr - page_start + MM_PAGESZ),
                           (void *) (src_addr + shdr->sh_offset),
                           shdr->sh_size);
                } else {
                    memset((void *) (shdr->sh_addr - page_start + MM_PAGESZ), 0, shdr->sh_size);
                }

                // Unmap page
                mm_unmap_cont_region(mm_kernel, 0x400000, 1, 0);
            }
        }
    }

    kdebug("DONE! entry is %p, welcome\n", entry_addr);

    return (int) entry_addr;
}
