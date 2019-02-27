#include "pm.h"
#include "sys/mm.h"
#include "sys/mem.h"
#include "arch/hw.h"
#include "sys/debug.h"
#include "multiboot.h"
#include "sys/panic.h"
#include "hw/hw.h"

#define PSTACK_SIZE     256
#define SSTACK_SIZE     4096

static uint32_t s_pstack[PSTACK_SIZE];
static uint32_t s_sstack[SSTACK_SIZE];
static uint32_t *s_psp = s_pstack + PSTACK_SIZE;
static uint32_t *s_ssp = s_sstack + PSTACK_SIZE;


static void x86_mm_claim_small(uintptr_t page) {
    if (s_ssp == s_sstack) {
        panic("No free pages left\n");
    }
    *(--s_ssp) = page;
}

void x86_mm_claim_page(uintptr_t page, int sz) {
    if (sz == 0x1000) {
        x86_mm_claim_small(page);
        return;
    }

    if (s_psp == s_pstack) {
        panic("No free pages left\n");
    }
    *(--s_psp) = page;
}

static int x86_mm_shatter() {
    if (s_psp == s_pstack + PSTACK_SIZE) {
        return -1;
    }

    uintptr_t a = *s_psp++;

    for (int i = 0; i < 1024; ++i) {
        x86_mm_claim_small(a + i * 0x1000);
    }

    return 0;
}

uintptr_t mm_alloc_phys_page(size_t sz) {
    if (sz == 0x1000) {
        kdebug("Alloc small page\n");

        if (s_ssp == s_sstack + SSTACK_SIZE) {
            // Try "shattering" huge page into small ones
            if (x86_mm_shatter() != 0) {
                return MM_NADDR;
            }

            return mm_alloc_phys_page(sz);
        }

        return *s_ssp++;
    } else {
        if (s_psp == s_pstack + PSTACK_SIZE) {
            return MM_NADDR;
        }

        return *s_psp++;
    }
}

void x86_pm_init(void) {
    // Add all physical memory entries
    struct multiboot_mmap_entry *mmap_first = (struct multiboot_mmap_entry *) (x86_multiboot_info->mmap_addr + KERNEL_VIRT_BASE);
    struct multiboot_mmap_entry *mmap = mmap_first;
    size_t mmap_len = x86_multiboot_info->mmap_length;
    int index = 0;
    int claimed_pages = 0;
    char siz_buf[11];

    kdebug("Memory map:\n");

    while (1) {
        uint32_t addr = (uint32_t) mmap->addr;
        uint32_t len = (uint32_t) mmap->len;

        fmtsiz(len, siz_buf);
        kdebug(" [%d] %c %p %10s\n", index++, mmap->type == MULTIBOOT_MEMORY_AVAILABLE ? '+' : '-', addr, siz_buf);

        // Claim 4M physical pages
        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
            uintptr_t aligned_addr = MM_ALIGN_UP(addr, 0x400000);
            uintptr_t diff = aligned_addr - addr;

            if (len > diff && len > 0x400000) {
                len -= diff;

                for (int i = 0; i < (len / 0x400000); ++i) {
                    kdebug("Claim %p\n", aligned_addr + i * 0x400000);
                    x86_mm_claim_page(aligned_addr + i * 0x400000, 0x400000);
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
    kdebug("Memory manager claimed %d pages (%s) of physical memory\n", claimed_pages, siz_buf);
}
