#include "sys/mm.h"
#include "sys/mem.h"
#include "sys/debug.h"
#include "sys/assert.h"
#include "arch/hw.h"

#define X86_MM_PAGE_POOL    (16)

static struct {
    uint32_t bitmap[1024 / 32];
    uint32_t index_last;
    uintptr_t vaddr;
    uintptr_t paddr;
} x86_page_pool[X86_MM_PAGE_POOL];

/// Allocate a single 4KiB page from paging structure pool
uintptr_t x86_page_pool_allocate(uintptr_t *phys) {
    for (int i = 0; i < X86_MM_PAGE_POOL; ++i) {
        // Paging pool is not initialized
        if (!x86_page_pool[i].vaddr) {
            x86_page_pool[i].paddr = mm_alloc_physical_page(MM_FLG_PS);
            x86_page_pool[i].vaddr = KERNEL_VIRT_BASE + (4 << 22) + (i << 22);
            x86_page_pool[i].index_last = 0;

            // Map vaddr -> paddr in kernel address space
            mm_map_range_pages(mm_kernel, x86_page_pool[i].vaddr, &x86_page_pool[i].paddr, 1, MM_FLG_PS | MM_FLG_WR);

            memsetl(x86_page_pool[i].bitmap, 0, 1024 / 32);
        }

        for (uint32_t j = x86_page_pool[i].index_last; j < 1024; ++j) {
            if (x86_page_pool[i].bitmap[j] == 0xFFFFFFFF) {
                continue;
            }

            for (uint32_t k = 0; k < 32; ++k) {
                if (!(x86_page_pool[i].bitmap[j] & ((uint32_t) 1 << k))) {
                    x86_page_pool[i].bitmap[j] |= (uint32_t) 1 << k;
                    x86_page_pool[i].index_last = j;
                    if (phys) {
                        *phys = x86_page_pool[i].paddr + (j << 17) + (k << 12);
                    }
                    kdebug("Allocated pool page\n");
                    return x86_page_pool[i].vaddr + (j << 17) + (k << 12);
                }
            }
        }
        for (uint32_t j = 0; j < x86_page_pool[i].index_last; ++j) {
            if (x86_page_pool[i].bitmap[j] == 0xFFFFFFFF) {
                continue;
            }

            for (uint32_t k = 0; k < 32; ++k) {
                if (!(x86_page_pool[i].bitmap[j] & ((uint32_t) 1 << k))) {
                    x86_page_pool[i].bitmap[j] |= (uint32_t) 1 << k;
                    x86_page_pool[i].index_last = j;
                    if (phys) {
                        *phys = x86_page_pool[i].paddr + (j << 17) + (k << 12);
                    }
                    kdebug("Allocated pool page\n");
                    return x86_page_pool[i].vaddr + (j << 17) + (k << 12);
                }
            }
        }
    }

    return MM_NADDR;
}

void x86_page_pool_free(uintptr_t vaddr) {
    for (int i = 0; i < X86_MM_PAGE_POOL; ++i) {
        // Lookup the pool the page belongs to
        if (x86_page_pool[i].vaddr) {
            if (x86_page_pool[i].vaddr <= vaddr && vaddr - x86_page_pool[i].vaddr < 0x400000) {
                uint32_t index = (vaddr - x86_page_pool[i].vaddr) >> 17;
                uint32_t bit = ((vaddr - x86_page_pool[i].vaddr) >> 12) & 0x1F;
                assert(x86_page_pool[i].bitmap[index] & ((uint32_t) 1 << bit));

                x86_page_pool[i].bitmap[index] &= ~((uint32_t) 1 << bit);
                return;
            }
        }
    }
    panic("The page does not belong to any of pools");
}

mm_space_t mm_create_space(uintptr_t *phys) {
    uintptr_t vaddr = x86_page_pool_allocate(phys);
    if (vaddr == MM_NADDR) {
        return NULL;
    }
    mm_space_t space = (mm_space_t) vaddr;
    memsetl(space, 0, 1024);

    uintptr_t table_info = x86_page_pool_allocate(NULL);
    memsetl((void *) table_info, 0, 1024);

    // The 0 entry of the directory is a virtual address of itself
    // So that we don't need to perform any reverse lookups of its
    // physical address.
    // Assuming `space` is already 0x1000-aligned, we can be sure that it
    // won't be available to user/kernel addressing, as 0th bit is 0.
    space[0] = (uintptr_t) space;

    // The 1023 entry is a virtual address of table information block
    space[1023] = table_info;

    return space;
}

void mm_destroy_space(mm_space_t pd) {
    x86_page_pool_free(pd[1023]);
    x86_page_pool_free((uintptr_t) pd);
}

////

void x86_mm_pool_init(void) {
    // Mark all pages in physical space as unavailable
    memset(x86_page_pool, 0, sizeof(x86_page_pool));
}
