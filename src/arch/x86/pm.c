#include "pm.h"
#include "sys/mm.h"
#include "sys/mem.h"
#include "arch/hw.h"
#include "sys/debug.h"
#include "multiboot.h"
#include "sys/panic.h"
#include "hw/hw.h"

#define PSTACK_SIZE     256

static uint32_t s_pstack[PSTACK_SIZE];
static uint32_t *s_psp = s_pstack + PSTACK_SIZE;

void x86_mm_claim_page(uintptr_t page) {
    if (s_psp == s_pstack) {
        panic("No free pages left\n");
    }
    *(--s_psp) = page;
}

static uintptr_t x86_mm_alloc_phys_page(uintptr_t start, uintptr_t end) {
    if (s_psp == s_pstack + PSTACK_SIZE) {
        return MM_NADDR;
    }

    return *s_psp++;
}

uintptr_t mm_alloc_phys_page(void) {
    return x86_mm_alloc_phys_page(0x400000, -0x400000);
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
    kdebug("Memory manager claimed %d pages (%s) of physical memory\n", claimed_pages, siz_buf);
}
