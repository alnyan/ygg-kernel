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
    if (index != 1023) {
        // Increase pagetable refcount (NYI)
        panic("4K-pages are not supported yet\n");
    }

    ++pd[1023];
}

int x86_mm_pddecr(mm_pagedir_t pd, uint32_t index) {
    if (index != 1023) {
        panic("4K-pages are not supported yet\n");
    }

    return --pd[1023] == 0;
}

static int x86_mm_map(mm_pagedir_t pd, uintptr_t virt_page, uintptr_t phys_page, uint32_t flags) {
#ifdef ENABLE_MAP_TRACE
    kdebug("map %p[%d (%p)] = %p, r%c, %c\n", pd, virt_page >> 22, virt_page, phys_page,
            (flags & X86_MM_FLG_RW) ? 'w' : 'o',
            (flags & X86_MM_FLG_US) ? 'u' : 'k');
#endif
    // Last page is used for refcounts
    assert((virt_page >> 22) != 1023);
    uint32_t pdi = virt_page >> 22;

    if (!(flags & X86_MM_FLG_PS)) {
        if (pd[pdi] & 1) {
            // Mapping already exists, map a page into the table
            mm_pagetab_t pt = (mm_pagetab_t) (x86_mm_reverse_lookup(pd[pdi] & -0x1000));
            assert((uintptr_t) pt != MM_NADDR);

            uint32_t pti = (virt_page >> 12) & 0x3FF;

            if (pt[pti] & X86_MM_FLG_PR) {
                if (flags & X86_MM_HNT_OVW) {
                    // TODO: trigger some unmap event?
                } else {
                    // Overwrite not allowed, return an error
                    panic("Trying to map the same location twice: %p\n", virt_page);
                }
            }

            pt[pti] = (phys_page & -0x1000) | X86_MM_FLG_PR | (flags & 0x3FF);

            return 0;
        } else {
            uintptr_t pt_phys;
            mm_pagetab_t pt = (mm_pagetab_t) mm_pagedir_alloc(&pt_phys); // TODO: mm_pagetab_alloc()
            assert(pt);
            assert((pt_phys & 0xFFF) == 0);

            uint32_t pti = (virt_page >> 12) & 0x3FF;

            pt[pti] = (phys_page & -0x1000) | X86_MM_FLG_PR | (flags & 0x3FF);
            pd[pdi] = pt_phys | X86_MM_FLG_PR | X86_MM_FLG_US | X86_MM_FLG_RW;

            return 0;
        }
    }


    if ((pd[pdi] & 0x1) != 0) {
        if (flags & X86_MM_HNT_OVW) {
            // TODO: trigger some unmap event?
        } else {
            // Overwrite not allowed, return an error
            panic("Trying to map the same location twice: %p\n", virt_page);
        }
    }

    // Perform the actual mapping
    pd[pdi] = (phys_page & -0x400000) | X86_MM_FLG_PR | (flags & 0x3FF);

    // Flush TLB by re-setting the same cr3
    asm volatile("movl %cr3, %eax; movl %eax, %cr3");
    return 0;
}

// TODO: merge x86_mm_map and mm_map_page
int mm_map_page(mm_pagedir_t pd, uintptr_t virt_page, uintptr_t phys_page, uint32_t flags) {
    uint32_t f = 0;
    if (flags & MM_FLG_RW) {
        f |= X86_MM_FLG_RW;
    }
    if (flags & MM_FLG_US) {
        f |= X86_MM_FLG_US;
    }
    if (flags & MM_FLG_HUGE) {
        f |= X86_MM_FLG_PS;
    }
    return x86_mm_map(pd, virt_page, phys_page, f);
}

uintptr_t mm_lookup(mm_pagedir_t pd, uintptr_t virt, uint32_t flags, uint32_t *rflags) {
    uint32_t pde = pd[virt >> 22];
    if (!(pde & X86_MM_FLG_PR)) {
        return MM_NADDR;
    }

    if (pde & X86_MM_FLG_PS) {
        if (!(flags & MM_FLG_HUGE)) {
            return MM_NADDR;
        }

        if (rflags) {
            *rflags = MM_FLG_HUGE | ((pde & X86_MM_FLG_PR) ? MM_FLG_RW : 0);
        }

        return (pde & -0x400000) | (virt & 0x3FFFFF);
    } else {
        if ((flags & MM_FLG_HUGE)) {
            return MM_NADDR;
        }


        mm_pagetab_t pt = (mm_pagetab_t) (x86_mm_reverse_lookup(pd[virt >> 22] & -0x1000));
        assert((uintptr_t) pt != MM_NADDR);

        uint32_t pti = (virt >> 12) & 0x3FF;
        uint32_t pte = pt[pti];

        if (pte & X86_MM_FLG_PR) {
            return (pte & -0x1000) | (virt & 0xFFF);
        }

        return MM_NADDR;
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
            panic("Trying to unmap a non-present page: %p\n", vaddr + (i << 22));
        } else {
#ifdef ENABLE_MAP_TRACE
            kdebug("unmap %p[%d (%p)]\n", pd, (vaddr >> 22) + i, vaddr + (i << 22));
#endif

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
    for (int i = 0; i < 1023; ++i) {
        if (pd[i] & X86_MM_FLG_PR) {
            if (pd[i] & X86_MM_FLG_PS) {
                kdebug("PD:%p[%d (%p)] = %p, r%c, %c\n", pd, i, i << 22, pd[i] & -0x400000,
                        pd[i] & X86_MM_FLG_RW ? 'w' : 'o', pd[i] & X86_MM_FLG_US ? 'u' : 'k');
            } else {
                kdebug("PD:%p[%d (%p)]: Table %p, r%c, %c\n", pd, i, i << 22, pd[i] & -0x1000,
                        pd[i] & X86_MM_FLG_RW ? 'w' : 'o', pd[i] & X86_MM_FLG_US ? 'u' : 'k');

                mm_pagedir_t pt = (mm_pagedir_t) x86_mm_reverse_lookup(pd[i] & -0x1000);
                assert(pt);

                for (int j = 0; j < 1024; ++j) {
                    if (pt[j] & X86_MM_FLG_PR) {
                        uintptr_t vaddr = (i << 22) + (j << 12);
                        kdebug(" PT:%p[%d (%p)] = %p, r%c, %c\n", pt, j, vaddr, pt[j] & -0x1000,
                                 pt[j] & X86_MM_FLG_RW ? 'w' : 'o', pt[j] & X86_MM_FLG_US ? 'u' : 'k');
                    }
                }
            }
        }
    }
}

void x86_mm_early_init(void) {
    extern uint32_t boot_page_directory[1024];
    mm_current = boot_page_directory;
    mm_kernel = mm_current;

    x86_pm_init();
}

void x86_mm_init(void) {
    // Dump entries retained from bootloader
    kdebug("Initializing memory management\n");

    // Add free space after the kernel to heap
    heap_init();

    extern void _kernel_end_virt();
    heap_add_region((uintptr_t) _kernel_end_virt, KERNEL_VIRT_BASE + 0x400000);

    // Setup page directory allocator
    x86_mm_alloc_init();
}
