#include <stdint.h>
#include "sys/debug.h"
#include "arch/hw.h"

#define X86_MM_FLG_PS   (1 << 7)
#define X86_MM_FLG_PR   (1 << 0)
#define X86_MM_HNT_OVW  (1 << 31)   // Kernel hint - overwrite existing mapping

typedef uintptr_t *mm_pagedir_t;
typedef uintptr_t *mm_pagetab_t;    // Not yet used

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

void x86_mm_init(void) {
    // Dump entries retained from bootloader
    debug("Initializing memory management\n");

    extern uint32_t boot_page_directory[1024];
    s_mm_current = boot_page_directory;
    s_mm_kernel = s_mm_current;

    for (int i = 0; i < 1024; ++i) {
        if (s_mm_current[i] & 0x1) {
            debug("retain: boot_page_directory[%d (0x%x)] = 0x%x\n", i, i << 22, s_mm_current[i] & -0x400000);
        }
    }
}
