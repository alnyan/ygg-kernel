#include "elf.h"
#include "sys/string.h"
#include "sys/panic.h"
#include "sys/debug.h"
#include "sys/mem.h"
#include "sys/mm.h"
#include <stddef.h>

// Loads ELF file into memory space `dst'
int elf_load(mm_pagedir_t dst, uintptr_t src_addr, size_t src_len) {
    debug("Trying to load ELF from %p, %uB\n", src_addr, src_len);

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *) src_addr;
    Elf32_Shdr *shdrs = (Elf32_Shdr *) (src_addr + ehdr->e_shoff);
    Elf32_Shdr *shstrtabh = &shdrs[ehdr->e_shstrndx];
    const char *shstrtabd = (const char *) (src_addr + shstrtabh->sh_offset);
    uint32_t entry_addr = ehdr->e_entry;

    // Validate ident
    if (strncmp(ehdr->e_ident, "\x7F" "ELF", 4)) {
        debug("\tError: identity doesn't match ELF ident\n");
        return -1;
    }

    debug("There are %u sections:\n", ehdr->e_shnum);

    for (size_t i = 0; i < ehdr->e_shnum; ++i) {
        Elf32_Shdr *shdr = &shdrs[i];
        const char *name = NULL;

        if (shdr->sh_name) {
            name = &shstrtabd[shdr->sh_name];
        }

        debug(" [%d] %s\n", i, name);

        // Load only program data
        if (shdr->sh_type == SHT_PROGBITS || shdr->sh_type == SHT_NOBITS) {
            if (shdr->sh_flags & SHF_ALLOC) {
                // Just alloc pages for the section
                // TODO: This is platform-specific code and needs to be moved somewhere
                uintptr_t page_start = shdr->sh_addr & -0x400000;
                uintptr_t page_end = MM_ALIGN_UP(shdr->sh_addr + shdr->sh_size, 0x400000);
                size_t page_count = (page_end - page_start) / 0x400000;

                if (page_count != 1) {
                    panic("NYI\n");
                }

                if (!(dst[(page_start) >> 22] & 1)) {
                    // Allocate a physical page
                    uintptr_t page = mm_alloc_phys_page();

                    if (page == MM_NADDR) {
                        panic("Failed to allocate memory for ELF\n");
                    }

                    x86_mm_map(dst, page_start, page, X86_MM_FLG_RW | X86_MM_FLG_PS | X86_MM_FLG_US);
                }

                // Map the page into kernel space
                // 0x400000 is the base for write access
                x86_mm_map(mm_kernel, 0x400000, dst[page_start >> 22] & -0x400000, X86_MM_FLG_RW | X86_MM_FLG_PS);

                if (shdr->sh_type == SHT_PROGBITS) {
                    memcpy(shdr->sh_addr - page_start + 0x400000, src_addr + shdr->sh_offset, shdr->sh_size);
                } else {
                    memset(shdr->sh_addr - page_start + 0x400000, 0, shdr->sh_size);
                }

                // Unmap page
                mm_unmap_cont_region(mm_kernel, 0x400000, 1, 0);
            }
        }
    }

    debug("DONE! entry is %p, welcome\n", entry_addr);

    return (int) entry_addr;
}
