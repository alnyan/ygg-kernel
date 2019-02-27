#include "sys/mm.h"
#include "arch/hw.h"
#include "sys/mem.h"
#include "sys/debug.h"
#include "sys/panic.h"
#include "sys/assert.h"

struct mm_alloc_block {
    uintptr_t vaddr;
    uintptr_t paddr;
};

static struct mm_alloc_block s_mm_alloc_blocks[4] = {0};
static size_t s_mm_alloc_dirs = 0;
static size_t s_mm_alloc_tabs = 0;
static size_t s_mm_alloc_free_dirs = 0;
static size_t s_mm_alloc_free_tabs = 0;

uintptr_t x86_mm_reverse_lookup(uintptr_t cr3) {
    if (cr3 == (uintptr_t) mm_kernel - KERNEL_VIRT_BASE) {
        return (uintptr_t) mm_kernel;
    }

    for (int i = 0; i < 4; ++i) {
        if (s_mm_alloc_blocks[i].vaddr == 0) {
            return MM_NADDR;
        }

        if (cr3 >= s_mm_alloc_blocks[i].paddr && cr3 < (s_mm_alloc_blocks[i].paddr + MM_PAGESZ_HUGE)) {
            return s_mm_alloc_blocks[i].vaddr - s_mm_alloc_blocks[i].paddr + cr3;
        }
    }

    return MM_NADDR;
}

void x86_mm_alloc_init(void) {
    uintptr_t page = mm_alloc_kernel_pages(mm_kernel, 1, MM_FLG_RW | MM_FLG_HUGE);
    kdebug("mmalloc page: %p\n", page);
    assert(page != 0xFFFFFFFF);
    memset((void *) (page + 0x1000 * 1023), 0, 4096);

    // Set up mapping blocks so we can map phys cr3s to virtual counterparts
    memset(s_mm_alloc_blocks, 0, sizeof(s_mm_alloc_blocks));
    s_mm_alloc_blocks[0].vaddr = page;
    s_mm_alloc_blocks[0].paddr = mm_kernel[page >> 22] & -0x400000;
}

void mm_clone(mm_pagedir_t dst, const mm_pagedir_t src) {
    // TODO: support pagedir nesting
    memcpy(dst, src, 1023 * 4);
}

static uintptr_t x86_mm_alloc_4k(uintptr_t *phys) {
    for (int j = 0; j < 4; ++j) {
        if (!s_mm_alloc_blocks[j].paddr) {
            return 0;
        }

        uintptr_t page = s_mm_alloc_blocks[j].vaddr;
        uint32_t *track = (uint32_t *) (page + 0x1000 * 1023);

        for (int i = 0; i < 1023; ++i) {
            if (track[i >> 5] & (1 << (i & 0x1F))) {
                continue;
            }

            track[i >> 5] |= 1 << (i & 0x1F);
            if (phys) {
                *phys = s_mm_alloc_blocks[j].paddr + i * 0x1000;
            }
            return i * 0x1000 + page;
        }
    }
    return 0;
}

// TODO: reference counting
mm_pagedir_t mm_pagedir_alloc(uintptr_t *phys) {
    uintptr_t res = x86_mm_alloc_4k(phys);
    if (res) {
        ++s_mm_alloc_tabs;
        memset((void *) res, 0, 0x1000);
    }
    return (mm_pagedir_t) res;
}

mm_pagetab_t x86_mm_pagetab_alloc(mm_pagedir_t parent, uintptr_t *phys) {
    uintptr_t res = x86_mm_alloc_4k(phys);
    if (res) {
        ++s_mm_alloc_tabs;
        memset((void *) res, 0, 0x1000);
    }
    return (mm_pagedir_t) res;
}

void mm_pagedir_free(mm_pagedir_t pd) {
    uint32_t cr3;
    asm volatile ("mov %%cr3, %0":"=a"(cr3));
    assert(cr3 != (uint32_t) pd - KERNEL_VIRT_BASE);

    uintptr_t page = MM_NADDR;

    for (int j = 0; j < 4; ++j) {
        if (!s_mm_alloc_blocks[j].vaddr) {
            break;
        }

        if (((uintptr_t) pd & -0x400000) == s_mm_alloc_blocks[j].vaddr) {
            page = s_mm_alloc_blocks[j].vaddr;
            break;
        }
    }

    assert(page != MM_NADDR);

    uint32_t *track = (uint32_t *) (page + 0x1000 * 1023);
    uint32_t i = ((uint32_t) pd & 0x3FFFFF) >> 12;

    if (!(track[i >> 5] & (1 << (i & 0x1F)))) {
        panic("Invalid pagedir free\n");
    }

    ++s_mm_alloc_free_dirs;
    track[i >> 5] &= ~(1 << (i & 0x1F));
}

void x86_mm_alloc_dump(void) {
    int c = 0;
    for (int i = 0; i < sizeof(s_mm_alloc_blocks) / sizeof(s_mm_alloc_blocks[0]); ++i) {
        if (!s_mm_alloc_blocks[i].paddr) {
            break;
        }

        ++c;
    }

    kdebug(" Block count: %u\n", c);

    for (int i = 0; i < c; ++i) {
        kdebug(" [%u] %p .. %p:\n",
                i,
                s_mm_alloc_blocks[i].vaddr,
                s_mm_alloc_blocks[i].vaddr + MM_PAGESZ_HUGE);
        kdebug("  (phys %p .. %p)\n",
                s_mm_alloc_blocks[i].paddr,
                s_mm_alloc_blocks[i].paddr + MM_PAGESZ_HUGE);
    }

    kdebug(" Dirs allocd: %u\n", s_mm_alloc_dirs);
    kdebug(" Freed: %u\n", s_mm_alloc_free_dirs);
    kdebug(" Tabs allocd: %u\n", s_mm_alloc_tabs);
    kdebug(" Freed: %u\n", s_mm_alloc_free_tabs);
}
