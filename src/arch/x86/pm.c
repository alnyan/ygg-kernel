#include "pm.h"
#include "sys/mm.h"
#include "sys/mem.h"
#include "arch/hw.h"
#include "sys/debug.h"
#include "multiboot.h"
#include "hw/hw.h"

// Physical page tracking
#define X86_MM_PHYS_PAGE    0x400000
#define X86_MM_PHYS_PAGES   1024
#define X86_MM_PTRACK_I(a)  ((a) / (X86_MM_PHYS_PAGE << 5))
#define X86_MM_PTRACK_N(a)  (((a) / X86_MM_PHYS_PAGE) & 0x1F)
static uint32_t s_ptrack[X86_MM_PHYS_PAGES >> 5];

void x86_mm_claim_page(uintptr_t page) {
    s_ptrack[X86_MM_PTRACK_I(page)] &= ~(1 << X86_MM_PTRACK_N(page));
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

uintptr_t mm_alloc_phys_page(void) {
    return x86_mm_alloc_phys_page(0x400000, -0x400000);
}

void x86_pm_init(void) {
    // Set allocation mask on all physical pages
    memset(s_ptrack, 0xFF, sizeof(s_ptrack));

    // Add all physical memory entries
    struct multiboot_mmap_entry *mmap_first = (struct multiboot_mmap_entry *) (x86_multiboot_info->mmap_addr + KERNEL_VIRT_BASE);
    struct multiboot_mmap_entry *mmap = mmap_first;
    size_t mmap_len = x86_multiboot_info->mmap_length;
    int index = 0;
    int claimed_pages = 0;
    char siz_buf[11];

    debug("Memory map:\n");

    while (1) {
        uint32_t addr = (uint32_t) mmap->addr;
        uint32_t len = (uint32_t) mmap->len;

        fmtsiz(len, siz_buf);
        debug(" [%d] %c %p %10s\n", index++, mmap->type == MULTIBOOT_MEMORY_AVAILABLE ? '+' : '-', addr, siz_buf);

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

    fmtsiz(claimed_pages * 0x400000, siz_buf);
    debug("Memory manager claimed %d pages (%s) of physical memory\n", claimed_pages, siz_buf);
}
