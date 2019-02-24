#pragma once

#define MM_NADDR    ((uintptr_t) 0xFFFFFFFF)
#define MM_PAGESZ   0x400000
#define MM_PAGESZ_SMALL 0x1000

#define X86_MM_FLG_PS   (1 << 7)
#define X86_MM_FLG_US   (1 << 2)
#define X86_MM_FLG_RW   (1 << 1)
#define X86_MM_FLG_PR   (1 << 0)
#define X86_MM_HNT_OVW  (1 << 31)   // Kernel hint - overwrite existing mapping

typedef uintptr_t *mm_pagedir_t;
typedef uintptr_t *mm_pagetab_t;    // Not yet used

extern mm_pagedir_t mm_kernel;
extern mm_pagedir_t mm_current;

void x86_mm_early_init(void);
void x86_mm_init(void);

void x86_mm_alloc_init(void);

void x86_mm_pdincr(mm_pagedir_t pd, uint32_t index);
int x86_mm_pddecr(mm_pagedir_t pd, uint32_t index);

uintptr_t x86_mm_reverse_lookup(uintptr_t cr3);
