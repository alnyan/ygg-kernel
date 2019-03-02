#include "sys/mm.h"
#include "sys/mem.h"
#include "sys/debug.h"
#include "multiboot.h"
#include "sys/assert.h"
#include "hw/hw.h"
#include "arch/hw.h"

#define X86_MM_FLG_PR       (1 << 0)
#define X86_MM_FLG_WR       (1 << 1)
#define X86_MM_FLG_US       (1 << 2)
#define X86_MM_FLG_PS       (1 << 7)

// Number of pages of addressable physical space. 1GiB for now.
#define X86_MM_PHYS_MAX     (262144)

// Kernel space internal subdivision
//  16MiB - kernel itself
#define X86_MM_PAGE_POOL    (16)
//  16MiB - paging structure pool

mm_space_t mm_kernel;

////

static struct {
    uint32_t bitmap[1024 / 32];
    uint32_t index_last;
    uintptr_t vaddr;
    uintptr_t paddr;
} x86_page_pool[X86_MM_PAGE_POOL];

/// Allocate a single 4KiB page from paging structure pool
static uintptr_t x86_page_pool_allocate(uintptr_t *phys) {
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

static void x86_page_pool_free(uintptr_t vaddr) {
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

static uint32_t x86_physical_page_bitmap[X86_MM_PHYS_MAX / 8];
static uintptr_t x86_physical_page_alloc_last = 0;
static size_t x86_physical_page_count = 0;

// Allocates a single 4KiB page
static uintptr_t x86_mm_alloc_physical_page(void) {
    // Pass 1 - start from last allocated page index
    for (uint32_t i = (x86_physical_page_alloc_last >> 17); i < X86_MM_PHYS_MAX / 8; ++i) {
        // 32 bits describe a 128KiB region of memory
        if (x86_physical_page_bitmap[i] == 0xFFFFFFFF) {
            // No free pages, skip
            continue;
        }

        for (uint8_t j = 0; j < 32; ++j) {
            if (!(x86_physical_page_bitmap[i] & ((uint32_t) 1 << j))) {
                uintptr_t page = (i << 17) | ((uint32_t) j << 12);
                x86_physical_page_bitmap[i] |= ((uint32_t) 1 << j);
                x86_physical_page_alloc_last = page + 0x1000;
                return page;
            }
        }
    }

    // Pass 2 - start from zero
    for (uint32_t i = 0; i < (x86_physical_page_alloc_last >> 17); ++i) {
        // 32 bits describe a 128KiB region of memory
        if (x86_physical_page_bitmap[i] == 0xFFFFFFFF) {
            // No free pages, skip
            continue;
        }

        for (uint8_t j = 0; j < 32; ++j) {
            if (!(x86_physical_page_bitmap[i] & ((uint32_t) 1 << j))) {
                uintptr_t page = (i << 17) | ((uint32_t) j << 12);
                x86_physical_page_bitmap[i] |= ((uint32_t) 1 << j);
                x86_physical_page_alloc_last = page + 0x1000;
                return page;
            }
        }
    }

    return MM_NADDR;
}

static uintptr_t x86_mm_alloc_physical_page_huge(void) {
    for (uint32_t i = 0; i < X86_MM_PHYS_MAX / 8; i += 32) {
        int f = 0;
        for (uint32_t j = 0; j < 32; ++j) {
            if (x86_physical_page_bitmap[i + j] != 0) {
                f = 1;
                break;
            }
        }
        if (!f) {
            uintptr_t page = (i << 17);
            x86_physical_page_alloc_last = page + 0x400000;
            return page;
        }
    }

    return MM_NADDR;
}

static void x86_mm_add_physical_page(uintptr_t page) {
    ++x86_physical_page_count;
    x86_physical_page_bitmap[page >> 17] &= ~((uint32_t) 1 << ((page >> 12) & 0x1F));
}

static int x86_mm_can_claim(uintptr_t page) {
    return ((page >> 12) < X86_MM_PHYS_MAX) && (page > (4 * 0x400000));
}

uintptr_t mm_alloc_physical_page(uint32_t flags) {
    if (flags & MM_FLG_PS) {
        return x86_mm_alloc_physical_page_huge();
    } else {
        return x86_mm_alloc_physical_page();
    }
}

void mm_free_physical_page(uintptr_t page, uint32_t flags) {
    if (flags & MM_FLG_PS) {
        x86_physical_page_count += 32;
        x86_physical_page_bitmap[page >> 17] = 0;
    } else {
        ++x86_physical_page_count;
        x86_physical_page_bitmap[page >> 17] &= ~((uint32_t) 1 << ((page >> 12) & 0x1F));
    }
}

////

int mm_map_range_pages(mm_space_t pd, uintptr_t start, uintptr_t *pages, size_t count, uint32_t flags) {
    if (flags & MM_FLG_PS) {
        uint32_t dst_flags = (flags & (MM_FLG_WR | MM_FLG_US)) | X86_MM_FLG_PS | X86_MM_FLG_PR;
        for (int i = 0; i < count; ++i) {
            assert(!(pd[(start >> 22) + i] & X86_MM_FLG_PR));
            kdebug("map %p[%d (%p)] = %p\n", pd, (start >> 22) + i, start + (i << 22), pages[i]);
            pd[(start >> 22) + i] = pages[i] | dst_flags;
        }
        return 0;
    } else {
        uint32_t dst_flags = (flags & (MM_FLG_WR | MM_FLG_US)) | X86_MM_FLG_PR;
        uint32_t *table_info = (uint32_t *) pd[1023];
        for (int i = 0; i < count; ++i) {
            uintptr_t vpage = start + i * 0x1000;
            uint32_t pdi = vpage >> 22;
            uint32_t pti = (vpage >> 12) & 0x3FF;

            uint32_t pde = pd[pdi];

            if (pde & X86_MM_FLG_PR) {
                // Lookup table address in table info structure
                assert(table_info[pdi]);
                mm_pagetab_t pt = (mm_pagetab_t) (table_info[pdi] & -0x1000);
                assert((table_info[pdi] & 0xFFF) != 0xFFF);
                ++table_info[pdi];

                kdebug("map %p[%d (%p)] = table %p[%d (%p)] = %p\n", pd, pdi, pdi << 22, pt, pti, vpage, pages[i]);
                pt[pti] = pages[i] | dst_flags;
            } else {
                uintptr_t phys;
                mm_pagetab_t pt = (mm_pagetab_t) x86_page_pool_allocate(&phys);
                assert((uintptr_t) pt != MM_NADDR);
                table_info[pdi] = (uintptr_t) pt | 1;

                kdebug("map %p[%d (%p)] = table %p\n", pd, pdi, pdi << 22, pt);
                pd[pdi] = phys | (X86_MM_FLG_PR | X86_MM_FLG_WR | X86_MM_FLG_US);

                kdebug("map %p[%d (%p)] = table %p[%d (%p)] = %p\n", pd, pdi, pdi << 22, pt, pti, vpage, pages[i]);
                pt[pti] = pages[i] | dst_flags;
            }
        }

        return 0;
    }

    return -1;
}

uintptr_t mm_map_range_linear(mm_space_t pd, uintptr_t start, uintptr_t pstart, size_t count, uint32_t flags) {
    if (flags & MM_FLG_PS) {
        start &= -0x400000;

        for (int i = 0; i < count; ++i) {
            uintptr_t ppage = pstart + i * 0x400000;
            if (mm_map_range_pages(pd, start + i * 0x400000, &ppage, 1, flags) != 0) {
                // TODO: free previously mapped pages
                return MM_NADDR;
            }
        }
    } else {
        start &= -0x1000;

        for (int i = 0; i < count; ++i) {
            uintptr_t ppage = pstart + i * 0x1000;
            if (mm_map_range_pages(pd, start + i * 0x1000, &ppage, 1, flags) != 0) {
                // TODO: free previously mapped pages
                return MM_NADDR;
            }
        }
    }

    return start;
}

int mm_umap_range(mm_space_t pd, uintptr_t start, size_t count, uint32_t flags) {
    if (flags & MM_FLG_PS) {
        for (int i = 0; i < count; ++i) {
            uint32_t pdi = (start >> 22) + i;
            assert(pd[(start >> 22) + i] & (X86_MM_FLG_PR | X86_MM_FLG_PS));
            kdebug("umap %p[%d (%p)]\n", pd, pdi, pdi << 22);

            if (!(flags & MM_FLG_NOPHYS)) {
                mm_free_physical_page(pd[pdi] & -0x400000, MM_FLG_PS);
            }

            pd[pdi] = 0;
        }
    } else {
        uint32_t *table_info = (uint32_t *) pd[1023];
        for (int i = 0; i < count; ++i) {
            uintptr_t vpage = start + i * 0x1000;
            uint32_t pdi = vpage >> 22;
            uint32_t pti = (vpage >> 12) & 0x3FF;

            uint32_t pde = pd[pdi];

            assert(pde & X86_MM_FLG_PR);
            assert(table_info[pdi]);
            mm_pagetab_t pt = (mm_pagetab_t) (table_info[pdi] & -0x1000);

            if (!(flags & MM_FLG_NOPHYS)) {
                mm_free_physical_page(pt[pti] & -0x1000, 0);
            }
            assert((table_info[pdi] & 0xFFF) != 0);
            kdebug("umap %p[%d (%p)] = table %p[%d (%p)]\n", pd, pdi, pdi << 22, pt, pti, vpage);
            pt[pti] = 0;
            if (((--table_info[pdi]) & 0xFFF) == 0) {
                kdebug("umap table %p[%d (%p)] = %p\n", pd, pdi, pdi << 22, pt);
                pd[pdi] = 0;
                table_info[pdi] = 0;
                x86_page_pool_free((uintptr_t) pt);
            }
        }
    }
    return -1;
}

////

void mm_dump_map(int level, mm_space_t pd) {
    kprint(level, "---- Address space dump ----\n");
    kprint(level, "L0 @ %p\n", pd);
    uint32_t *table_info = (uint32_t *) pd[1023];
    for (uint32_t pdi = 0; pdi < 1023; ++pdi) {
        if (pd[pdi] & X86_MM_FLG_PR) {
            if (pd[pdi] & X86_MM_FLG_PS) {
                kprint(level, "\t%p .. %p -> %p\n", pdi << 22, (pdi << 22) + 0x400000, pd[pdi] & -0x400000);
            } else {
                kprint(level, "\t%p .. %p -> L1 %p\n", pdi << 22, (pdi << 22) + 0x400000, pd[pdi] & -0x1000);
                mm_pagetab_t pt = (mm_pagetab_t) (table_info[pdi] & -0x1000);
                for (uint32_t pti = 0; pti < 1024; ++pti) {
                    if (pt[pti] & X86_MM_FLG_PR) {
                        kprint(level, "\t\t%p .. %p -> %p\n", (pdi << 22) | (pti << 12), ((pdi << 22) | (pti << 12)) + 0x1000, pt[pti] & -0x1000);
                    }
                }
            }
        }
    }
}

void mm_init(void) {
    // Mark all pages in physical space as unavailable
    memsetl(x86_physical_page_bitmap, 0xFFFFFFFF, sizeof(x86_physical_page_bitmap) / 4);
    memset(x86_page_pool, 0, sizeof(x86_page_pool));

    kdebug("Physical memory layout:\n");
    static const char *mmap_memory_types[] = {
        NULL,
        "Available",
        "Reserved",
        "ACPI",
        "NVS",
        "Bad RAM"
    };
    uint32_t mmap_usable_pages = 0;
    uintptr_t begin = 0xFFFFFFFF;
    uintptr_t end = 0;
    // Add physical pages from multiboot info
    for (struct multiboot_mmap_entry *it = (struct multiboot_mmap_entry *) (KERNEL_VIRT_BASE + x86_multiboot_info->mmap_addr);
         (uintptr_t) it < KERNEL_VIRT_BASE + x86_multiboot_info->mmap_addr + x86_multiboot_info->mmap_length;
         it = (struct multiboot_mmap_entry *) ((uintptr_t) it + it->size + sizeof(it->size))) {
        assert(it->type < 5);

        kdebug("%lp .. %lp: %s\n", it->addr, it->len + it->addr, mmap_memory_types[it->type]);

        if (it->type == MULTIBOOT_MEMORY_AVAILABLE) {
            uint32_t region_npages = (uint32_t) it->len / 0x1000;

            for (uint32_t i = 0; i < region_npages; ++i) {
                uint32_t page = (i << 12) + (uint32_t) (it->addr & -0x1000);

                if (x86_mm_can_claim(page)) {
                    if (page < begin) {
                        begin = page;
                    }
                    if (page > end) {
                        end = page;
                    }

                    x86_mm_add_physical_page(page);
                    ++mmap_usable_pages;
                }
            }
        }
    }

    char sizbuf[12];
    fmtsiz(mmap_usable_pages << 12, sizbuf);
    kinfo("Usable physical memory: %s\n", sizbuf);
    kdebug("Least addr: %p, Highest addr: %p\n", begin, end);

    // Set mm_kernel to page directory from boot stage
    extern uint32_t boot_page_directory[1024];
    mm_kernel = boot_page_directory;

    // Attach table_info structure to mm_kernel
    uint32_t *kernel_table_info = (uint32_t *) x86_page_pool_allocate(NULL);
    assert((uintptr_t) kernel_table_info != MM_NADDR);
    memsetl(kernel_table_info, 0, 1024);
    mm_kernel[1023] = (uint32_t) kernel_table_info;
    mm_kernel[0] = (uint32_t) mm_kernel;

    mm_dump_map(DEBUG_DEFAULT, mm_kernel);
}
