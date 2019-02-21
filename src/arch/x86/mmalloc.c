#include "sys/mm.h"
#include "arch/hw.h"
#include "sys/mem.h"
#include "sys/debug.h"
#include "sys/panic.h"
#include "sys/assert.h"

static uint32_t *s_mm_alloc_track;

void x86_mm_alloc_init(void) {
    uintptr_t page = mm_alloc_phys_page();
    assert(page != 0xFFFFFFFF);

    x86_mm_map(mm_kernel, KERNEL_VIRT_BASE + 0x400000, page, X86_MM_FLG_PS | X86_MM_FLG_RW);

    s_mm_alloc_track = (uint32_t *) (KERNEL_VIRT_BASE + 0x400000 + 0x1000 * 1023);
    memset(s_mm_alloc_track, 0, 4096);
}

void mm_clone(mm_pagedir_t dst, const mm_pagedir_t src) {
    memcpy(dst, src, 4096);
}

mm_pagedir_t mm_pagedir_alloc(void) {
    for (int i = 0; i < 1023; ++i) {
        if (s_mm_alloc_track[i >> 5] & (1 << (i & 0x1F))) {
            continue;
        }

        s_mm_alloc_track[i >> 5] |= 1 << (i & 0x1F);
        return (mm_pagedir_t) (i * 0x1000 + KERNEL_VIRT_BASE + 0x400000);
    }
    return NULL;
}

void mm_pagedir_free(mm_pagedir_t pd) {
    uint32_t cr3;
    asm volatile ("mov %%cr3, %0":"=a"(cr3));
    assert(cr3 != (uint32_t) pd - KERNEL_VIRT_BASE);
    uint32_t i = ((uint32_t) pd - KERNEL_VIRT_BASE - 0x400000) >> 12;
    if (!(s_mm_alloc_track[i >> 5] & (1 << (i & 0x1F)))) {
        panic("Invalid pagedir free\n");
    }

    s_mm_alloc_track[i >> 5] &= ~(1 << (i & 0x1F));
}
