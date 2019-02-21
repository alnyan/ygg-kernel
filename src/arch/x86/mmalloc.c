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

//static uint32_t *s_mm_alloc_track;
static struct mm_alloc_block s_mm_alloc_blocks[4];

uintptr_t x86_mm_reverse_lookup(uintptr_t cr3) {
    if (cr3 == (uintptr_t) mm_kernel - KERNEL_VIRT_BASE) {
        return (uintptr_t) mm_kernel;
    }

    for (int i = 0; i < 4; ++i) {
        if (s_mm_alloc_blocks[i].vaddr == 0) {
            return MM_NADDR;
        }

        if (cr3 >= s_mm_alloc_blocks[i].paddr && cr3 < (s_mm_alloc_blocks[i].paddr + 0x400000)) {
            return s_mm_alloc_blocks[i].vaddr - s_mm_alloc_blocks[i].paddr + cr3;
        }
    }

    return MM_NADDR;
}

void x86_mm_alloc_init(void) {
    uintptr_t page = mm_alloc_kernel_pages(mm_kernel, 1, MM_AFLG_RW);
    debug("x86_mm_alloc_init = %p\n", page);
    assert(page != 0xFFFFFFFF);

//    x86_mm_map(mm_kernel, KERNEL_VIRT_BASE + 0x400000, page, X86_MM_FLG_PS | X86_MM_FLG_RW);

    memset((void *) (page + 0x1000 * 1023), 0, 4096);

    // Set up mapping blocks so we can map phys cr3s to virtual counterparts
    memset(s_mm_alloc_blocks, 0, sizeof(s_mm_alloc_blocks));
    s_mm_alloc_blocks[0].vaddr = page;
    s_mm_alloc_blocks[0].paddr = mm_kernel[page >> 22] & -0x400000;
}

void mm_clone(mm_pagedir_t dst, const mm_pagedir_t src) {
    // TODO: support pagedir nesting
    debug("mm_clone %p <- %p\n", dst, src);
    debug("mm_clone_phys %p <- %p\n", mm_kernel[(uintptr_t) dst >> 22] & -0x400000, 0);
    memcpy(dst, src, 1023 * 4);
}

mm_pagedir_t mm_pagedir_alloc(void) {
    for (int j = 0; j < 4; ++j) {
        if (!s_mm_alloc_blocks[j].paddr) {
            return NULL;
        }

        uintptr_t page = s_mm_alloc_blocks[j].vaddr;
        uint32_t *track = (uint32_t *) (page + 0x1000 * 1023);

        for (int i = 0; i < 1023; ++i) {
            if (track[i >> 5] & (1 << (i & 0x1F))) {
                continue;
            }

            track[i >> 5] |= 1 << (i & 0x1F);
            mm_pagedir_t pd = (mm_pagedir_t) (i * 0x1000 + page);
            pd[1023] = 0;
            x86_mm_pdincr(pd, 1023);
            return pd;
        }
    }
    return NULL;
}

void mm_pagedir_free(mm_pagedir_t pd) {
    uint32_t cr3;
    asm volatile ("mov %%cr3, %0":"=a"(cr3));
    assert(cr3 != (uint32_t) pd - KERNEL_VIRT_BASE);
    debug("mm_pagedir_free = %p\n", pd);

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

    debug("page = %p\n", page);

    assert(page != MM_NADDR);

    uint32_t *track = (uint32_t *) (page + 0x1000 * 1023);
    uint32_t i = ((uint32_t) pd & 0x3FFFFF) >> 12;

    if (!(track[i >> 5] & (1 << (i & 0x1F)))) {
        panic("Invalid pagedir free\n");
    }

    if (x86_mm_pddecr(pd, 1023)) {
        track[i >> 5] &= ~(1 << (i & 0x1F));
    }
}
