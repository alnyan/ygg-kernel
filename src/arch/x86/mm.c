#include <stdint.h>
#include <stddef.h>
#include "sys/debug.h"
#include "sys/assert.h"
#include "sys/mem.h"
#include "sys/mm.h"
#include "def.h"
#include "mm.h"
#include "hw/hw.h"
#include "sys/heap.h"
#include "pm.h"

mm_pagedir_t mm_current;   // Currently used page directory
mm_pagedir_t mm_kernel;    // Kernel global page dir

void mm_set(mm_pagedir_t pd) {
    mm_current = pd;

    if (pd == mm_kernel) {
        uint32_t addr = (uint32_t) pd - KERNEL_VIRT_BASE;
        asm volatile ("mov %0, %%cr3"::"a"(addr));
        return;
    }

    // Lookup physical address
    if (mm_kernel[(uintptr_t) pd >> 22] & 1) {
        uint32_t phys = mm_kernel[(uintptr_t) pd >> 22] & -0x400000;

        asm volatile ("mov %0, %%cr3"::"a"(phys));
    } else {
        panic("Trying to bind non-existent PD\n");
    }
}

void x86_mm_pdincr(mm_pagedir_t pd, uint32_t index) {
    debug("increase refcount for %p\n", pd);
    if (index != 1023) {
        // Increase pagetable refcount (NYI)
        panic("4K-pages are not supported yet\n");
    }

    ++pd[1023];
}

int x86_mm_pddecr(mm_pagedir_t pd, uint32_t index) {
    debug("decrease refcount for %p\nb", pd);
    if (index != 1023) {
        panic("4K-pages are not supported yet\n");
    }

    return --pd[1023] == 0;
}

int x86_mm_map(mm_pagedir_t pd, uintptr_t virt_page, uintptr_t phys_page, uint32_t flags) {
    debug("map %p[%d (%p)] = %p, r%c, %c\n", pd, virt_page >> 22, virt_page, phys_page,
            (flags & X86_MM_FLG_RW) ? 'w' : 'o',
            (flags & X86_MM_FLG_US) ? 'u' : 'k');
    // Last page is used for refcounts
    assert((virt_page >> 22) != 1023);

    if (!(flags & X86_MM_FLG_PS)) {
        // TODO: panic!
        panic("4K-pages are not supported yet\n");
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
    pd[pdi] = (phys_page & -0x400000) | X86_MM_FLG_PR | (flags & 0x3FF);

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

static uintptr_t x86_mm_find_cont_region(mm_pagedir_t pd, uintptr_t start, uintptr_t end, int count) {
    uintptr_t addr = start & -0x400000;

    while (addr <= end - count * 0x400000) {
        int match = 1;

        for (int i = 0; i < count; ++i) {
            if (pd[(addr + i * 0x400000) >> 22] & X86_MM_FLG_PS) {
                match = 0;
                break;
            }
        }

        if (match) {
            return addr;
        }

        addr += 0x400000;
    }

    return MM_NADDR;
}

void mm_unmap_cont_region(mm_pagedir_t pd, uintptr_t vaddr, int count, uint32_t flags) {
    for (int i = 0; i < count; ++i) {
        uint32_t ent = pd[(vaddr >> 22) + i];

        if (!(ent & X86_MM_FLG_PR)) {
            debug("Trying to unmap a non-present page: %p\n", vaddr + (i << 22));
        } else {
            debug("unmap %p[%d (%p)]\n", pd, (vaddr >> 22) + i, vaddr + (i << 22));

            if (flags & MM_UFLG_PF) {
                x86_mm_claim_page(ent & -0x400000);
            }

            pd[(vaddr >> 22) + i] = 0;
        }
    }
}

uintptr_t mm_alloc_kernel_pages(mm_pagedir_t pd, int count, uint32_t flags) {
    uintptr_t vaddr = x86_mm_find_cont_region(pd, KERNEL_VIRT_BASE + 0xC00000, -0x400000, count);

    if (vaddr == MM_NADDR) {
        return MM_NADDR;
    }

    uint32_t xflags = X86_MM_FLG_PS;

    if (flags & MM_AFLG_US) {
        xflags |= X86_MM_FLG_US;
    }

    if (flags & MM_AFLG_RW) {
        xflags |= X86_MM_FLG_RW;
    }

    // Alloc and bind physical pages
    for (int i = 0; i < count; ++i) {
        //uintptr_t page = x86_mm_alloc_phys_page(0x400000, -0x400000);
        uintptr_t page = mm_alloc_phys_page();

        if (page == MM_NADDR) {
            // XXX: unmap previously mapped on failure
            return MM_NADDR;
        }

        x86_mm_map(pd, vaddr + i * 0x400000, page, xflags);
    }

    return vaddr;
}

void mm_dump_pages(mm_pagedir_t pd) {
    for (int i = 0; i < 1024; ++i) {
        if (pd[i] & X86_MM_FLG_PR) {
            x86_mm_dump_entry(pd, i);
        }
    }
}

void x86_mm_init(void) {
    // Dump entries retained from bootloader
    debug("Initializing memory management\n");

    x86_pm_init();

    extern uint32_t boot_page_directory[1024];
    mm_current = boot_page_directory;
    mm_kernel = mm_current;

    for (int i = 0; i < 1024; ++i) {
        if (mm_current[i] & 0x1) {
            x86_mm_dump_entry(mm_current, i);
        }
    }

    // Add free space after the kernel to heap
    heap_init();

    extern void _kernel_end_virt();
    heap_add_region((uintptr_t) _kernel_end_virt, KERNEL_VIRT_BASE + 0x400000);

    // Setup page directory allocator
    x86_mm_alloc_init();
}
