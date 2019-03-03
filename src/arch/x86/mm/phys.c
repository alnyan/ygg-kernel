#include "sys/mm.h"
#include "sys/mem.h"
#include "sys/debug.h"
#include "sys/assert.h"
#include "arch/hw.h"
#include "../hw/hw.h"
#include "../multiboot.h"

// Number of pages of addressable physical space. 1GiB for now.
#define X86_MM_PHYS_MAX     (262144)

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
        assert(x86_physical_page_bitmap[page >> 17] == 0xFFFFFFFF);
        x86_physical_page_count += 32;
        x86_physical_page_bitmap[page >> 17] = 0;
    } else {
        assert((x86_physical_page_bitmap[page >> 17] & ((uint32_t) 1 << ((page >> 12) & 0x1F))));
        ++x86_physical_page_count;
        x86_physical_page_bitmap[page >> 17] &= ~((uint32_t) 1 << ((page >> 12) & 0x1F));
    }
}

void x86_mm_phys_init(void) {
    memsetl(x86_physical_page_bitmap, 0xFFFFFFFF, sizeof(x86_physical_page_bitmap) / 4);

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
                    x86_mm_add_physical_page(page);
                    ++mmap_usable_pages;
                }
            }
        }
    }

    char sizbuf[12];
    fmtsiz(mmap_usable_pages << 12, sizbuf);
    kinfo("Usable physical memory: %s\n", sizbuf);
}
