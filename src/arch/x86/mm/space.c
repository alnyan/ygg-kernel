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
    uintptr_t phys2;
    uintptr_t vaddr = x86_page_pool_allocate(&phys2);
    if (phys) {
        *phys = phys2;
    }
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
    space[0] = (uintptr_t) phys2;

    // The 1023 entry is a virtual address of table information block
    space[1023] = table_info;

    return space;
}

void mm_destroy_space(mm_space_t pd) {
    x86_page_pool_free(pd[1023]);
    x86_page_pool_free((uintptr_t) pd);
}

////

void mm_space_clone(mm_space_t dst, const mm_space_t src, uint32_t flags) {
    memsetl(&dst[1], 0, 1022);

    uint32_t *src_table_info = (uint32_t *) src[1023];
    uint32_t *dst_table_info = (uint32_t *) dst[1023];

    memsetl(dst_table_info, 0, 1024);

    if (flags & MM_FLG_CLONE_KERNEL) {
        for (uint32_t pdi = KERNEL_VIRT_BASE >> 22; pdi < 1024; ++pdi) {
            if (!(src[pdi] & (1 << 0) /* X86_MM_FLG_PR */)) {
                continue;
            }

            if (src[pdi] & (1 << 7) /* X86_MM_FLG_PS */) {
                dst[pdi] = src[pdi];
            } else {
                assert(src_table_info[pdi]);
                mm_pagetab_t src_pt = (mm_pagetab_t) (src_table_info[pdi] & -0x1000);
                uintptr_t dst_pt_phys;
                mm_pagetab_t dst_pt = (mm_pagetab_t) x86_page_pool_allocate(&dst_pt_phys);
                assert((uintptr_t) dst_pt != MM_NADDR);

                dst_table_info[pdi] = ((uintptr_t) dst_pt) | (src_table_info[pdi] & 0xFFF);
                dst[pdi] = dst_pt_phys | ((1 << 0) | (1 << 1) | (1 << 2) /* X86_MM_FLG_PR | X86_MM_FLG_WR | X86_MM_FLG_US */);
                memcpy(dst_pt, src_pt, 4096);
            }
        }
    }
}

#if defined(ENABLE_TASK)
int mm_space_fork(mm_space_t dst, const mm_space_t src, uint32_t flags) {
    if (flags & MM_FLG_CLONE_KERNEL) {
        // Kernel pages do not need to be copied
        mm_space_clone(dst, src, MM_FLG_CLONE_KERNEL);
    }

    mm_dump_map(DEBUG_DEFAULT, src);
    uint32_t *src_table_info = (uint32_t *) src[1023];

    if (flags & MM_FLG_CLONE_USER) {
        // For now, just physically copy the pages
        for (uint32_t pdi = 1; pdi < KERNEL_VIRT_BASE >> 22; ++pdi) {
            if (!(src[pdi] & (1 << 0) /* X86_MM_FLG_PR */)) {
                continue;
            }

            // There shouldn't be any huge pages mapped in userspace
            assert(!(src[pdi] & (1 << 7) /* X86_MM_FLG_PS */));

            assert(src_table_info[pdi]);
            mm_pagetab_t src_pt = (mm_pagetab_t) (src_table_info[pdi] & -0x1000);

            for (uint32_t pti = 0; pti < 1024; ++pti) {
                if (src_pt[pti] & (1 << 0) /* X86_MM_FLG_PR */) {
                    uintptr_t vpage = (pdi << 22) | (pti << 12);

                    assert(mm_map_range(dst, vpage, 1, (src_pt[pti] & MM_FLG_WR) | MM_FLG_US) == 0);

                    // TODO: don't be lazy here, implement an user-to-user copy
                    static char tmp_buf[4096];
                    assert(mm_memcpy_user_to_kernel(src, tmp_buf, (const void *) vpage, 0x1000) == 0);
                    assert(mm_memcpy_kernel_to_user(dst, (void *) vpage, tmp_buf, 0x1000) == 0);
                }
            }
        }
    }

    return 0;
}
#endif

////

void x86_mm_pool_init(void) {
    // Mark all pages in physical space as unavailable
    memset(x86_page_pool, 0, sizeof(x86_page_pool));
}
