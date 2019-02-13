#include <stdint.h>
#include <stddef.h>
#include "sys/debug.h"
#include "arch/hw.h"
#include "mm.h"
#include "hw.h"

#define X86_MM_FLG_PS   (1 << 7)
#define X86_MM_FLG_US   (1 << 2)
#define X86_MM_FLG_RW   (1 << 1)
#define X86_MM_FLG_PR   (1 << 0)
#define X86_MM_HNT_OVW  (1 << 31)   // Kernel hint - overwrite existing mapping

static mm_pagedir_t s_mm_current;   // Currently used page directory
static mm_pagedir_t s_mm_kernel;    // Kernel global page dir

int x86_mm_map(mm_pagedir_t pd, uintptr_t virt_page, uintptr_t phys_page, uint32_t flags) {
    if (!(flags & X86_MM_FLG_PS)) {
        // TODO: panic!
        debug("PANIC: Non-large pages are not supported yet\n");
        while (1) {
            asm volatile ("cli; hlt");
        }
    }

    uint32_t pdi = virt_page >> 22;

    if ((pd[pdi] & 0x1) != 0) {
        if (flags & X86_MM_HNT_OVW) {
            // TODO: trigger some unmap event?
        } else {
            // Overwrite not allowed, return an error
            return -1;
        }
    }

    // Perform the actual mapping
    pd[pdi] = (pdi << 22) | X86_MM_FLG_PR | X86_MM_FLG_PS | (flags & 0x3FF);

    // Flush TLB by re-setting the same cr3
    asm volatile("movl %cr3, %eax; movl %eax, %cr3");
    return 0;
}

void x86_mm_dump_entry(mm_pagedir_t pd, uint32_t pdi) {
    if (pd[pdi] & 0x1) {
        debug("PD:%p[%d (%p)] = %p, r%c, %c\n", pd, pdi, pdi << 22, pd[pdi] & -0x400000,
                pd[pdi] & X86_MM_FLG_RW ? 'w' : 'o', pd[pdi] & X86_MM_FLG_US ? 'u' : 'k');
    } else {
        debug("PD:%p[%d (%p)] = not present\n", pd, pdi, pdi << 22);
    }
}

void x86_mm_init(void) {
    // Dump entries retained from bootloader
    debug("Initializing memory management\n");

    // Add all physical memory entries
    struct multiboot_mmap_entry *mmap_first = (struct multiboot_mmap_entry *) (x86_multiboot_info->mmap_addr + KERNEL_VIRT_BASE);
    struct multiboot_mmap_entry *mmap = mmap_first;
    size_t mmap_len = x86_multiboot_info->mmap_length;
    int index = 0;

    debug("Memory map:\n");

    while (1) {
        uint32_t addr = (uint32_t) mmap->addr;
        uint32_t len = (uint32_t) mmap->len;

        debug(" [%d] %c %p, %dK\n", index++, mmap->type == MULTIBOOT_MEMORY_AVAILABLE ? '+' : '-', addr, len / 1024);

        uintptr_t next_ptr = ((uintptr_t) mmap) + sizeof(mmap->size) + mmap->size;
        uintptr_t next_off = next_ptr - ((uintptr_t) mmap_first);

        if (next_off >= mmap_len) {
            break;
        }

        mmap = (struct multiboot_mmap_entry *) next_ptr;
    }

    extern uint32_t boot_page_directory[1024];
    s_mm_current = boot_page_directory;
    s_mm_kernel = s_mm_current;

    for (int i = 0; i < 1024; ++i) {
        if (s_mm_current[i] & 0x1) {
            x86_mm_dump_entry(s_mm_current, i);
        }
    }
}
