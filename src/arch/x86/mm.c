#include <stdint.h>
#include "sys/debug.h"
#include "arch/hw.h"
#include "mm.h"

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
        debug("PD:0x%x[%d (0x%x)] = 0x%x, r%c, %c\n", pd, pdi, pdi << 22, pd[pdi] & -0x400000,
                pd[pdi] & X86_MM_FLG_RW ? 'w' : 'o', pd[pdi] & X86_MM_FLG_US ? 'u' : 'k');
    } else {
        debug("PD:0x%x[%d (0x%x)] = not present\n", pd, pdi, pdi << 22);
    }
}

void x86_mm_init(void) {
    // Dump entries retained from bootloader
    debug("Initializing memory management\n");

    extern uint32_t boot_page_directory[1024];
    s_mm_current = boot_page_directory;
    s_mm_kernel = s_mm_current;

    for (int i = 0; i < 1024; ++i) {
        if (s_mm_current[i] & 0x1) {
            x86_mm_dump_entry(s_mm_current, i);
        }
    }
}
