#include <stdint.h>
#include <stddef.h>
#include "sys/debug.h"
#include "sys/mem.h"
#include "sys/mm.h"
#include "def.h"
#include "mm.h"
#include "hw.h"

mm_pagedir_t mm_current;   // Currently used page directory
mm_pagedir_t mm_kernel;    // Kernel global page dir

// Physical page tracking
#define X86_MM_PHYS_PAGE    0x400000
#define X86_MM_PHYS_PAGES   1024
#define X86_MM_PTRACK_I(a)  ((a) / (X86_MM_PHYS_PAGE << 5))
#define X86_MM_PTRACK_N(a)  (((a) / X86_MM_PHYS_PAGE) & 0x1F)
static uint32_t s_ptrack[X86_MM_PHYS_PAGES >> 5];

int x86_mm_map(mm_pagedir_t pd, uintptr_t virt_page, uintptr_t phys_page, uint32_t flags) {
    debug("map %p[%d (%p)] = %p, r%c, %c\n", pd, virt_page >> 22, virt_page, phys_page,
            (flags & X86_MM_FLG_RW) ? 'w' : 'o',
            (flags & X86_MM_FLG_US) ? 'u' : 'k');

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

static void x86_mm_claim_page(uintptr_t page) {
    s_ptrack[X86_MM_PTRACK_I(page)] &= ~(1 << X86_MM_PTRACK_N(page));
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

static uintptr_t x86_mm_alloc_phys_page(uintptr_t start, uintptr_t end) {
    debug("alloc phys page =\n");
    for (uintptr_t addr = start; addr < end - 0x400000; addr += 0x400000) {
        uint32_t bit = (1 << X86_MM_PTRACK_N(addr));
        uint32_t index = X86_MM_PTRACK_I(addr);

        if (!(s_ptrack[index] & bit)) {
            s_ptrack[index] |= bit;
            debug(" %p\n", addr);
            return addr;
        }
    }
    debug(" MM_NADDR\n");
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
    uintptr_t vaddr = x86_mm_find_cont_region(pd, KERNEL_VIRT_BASE + 0x400000, -0x400000, count);

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
        uintptr_t page = x86_mm_alloc_phys_page(0x400000, -0x400000);

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

    // Set allocation mask on all physical pages
    memset(s_ptrack, 0xFF, sizeof(s_ptrack));

    // Add all physical memory entries
    struct multiboot_mmap_entry *mmap_first = (struct multiboot_mmap_entry *) (x86_multiboot_info->mmap_addr + KERNEL_VIRT_BASE);
    struct multiboot_mmap_entry *mmap = mmap_first;
    size_t mmap_len = x86_multiboot_info->mmap_length;
    int index = 0;
    int claimed_pages = 0;

    debug("Memory map:\n");

    while (1) {
        uint32_t addr = (uint32_t) mmap->addr;
        uint32_t len = (uint32_t) mmap->len;

        debug(" [%d] %c %p, %dK\n", index++, mmap->type == MULTIBOOT_MEMORY_AVAILABLE ? '+' : '-', addr, len / 1024);

        // Claim 4M physical pages
        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
            uintptr_t aligned_addr = MM_ALIGN_UP(addr, 0x400000);
            uintptr_t diff = aligned_addr - addr;

            if (len > diff) {
                len -= diff;

                for (int i = 0; i < (len / 0x400000); ++i) {
                    x86_mm_claim_page(aligned_addr + i * 0x400000);
                    ++claimed_pages;
                }
            }
        }

        uintptr_t next_ptr = ((uintptr_t) mmap) + sizeof(mmap->size) + mmap->size;
        uintptr_t next_off = next_ptr - ((uintptr_t) mmap_first);

        if (next_off >= mmap_len) {
            break;
        }

        mmap = (struct multiboot_mmap_entry *) next_ptr;
    }

    debug("Memory manager claimed %d pages (%uM) of physical memory\n", claimed_pages, claimed_pages * 4);

    extern uint32_t boot_page_directory[1024];
    mm_current = boot_page_directory;
    mm_kernel = mm_current;

    for (int i = 0; i < 1024; ++i) {
        if (mm_current[i] & 0x1) {
            x86_mm_dump_entry(mm_current, i);
        }
    }
}
