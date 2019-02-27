#pragma once
#include <stdint.h>
#include <stddef.h>

#define MM_ALIGN_UP(p, a)   (((p) + (a) - 1) & ~((a) - 1))

#define MM_FLG_RW   (1 << 0)
#define MM_FLG_US   (1 << 1)
#define MM_FLG_HUGE  (1 << 2)

// Alloc flags
// 0x01: writable
#define MM_AFLG_RW   (1 << 0)
// 0x02: user-available
#define MM_AFLG_US   (1 << 1)

// Unmap flags
// 0x01: free physical pages
#define MM_UFLG_PF   (1 << 0)

#if defined(ARCH_X86)
#include "arch/x86/mm.h"
#endif

uintptr_t mm_alloc_kernel_pages(mm_pagedir_t pd, int count, uint32_t aflags);
uintptr_t mm_alloc_phys_page(void);
void mm_unmap_cont_region(mm_pagedir_t pd, uintptr_t addr, int count, uint32_t uflags);
void mm_dump_pages(mm_pagedir_t pd);

int mm_map_page(mm_pagedir_t pd, uintptr_t vaddr, uintptr_t paddr, uint32_t flags);
uintptr_t mm_lookup(mm_pagedir_t pd, uintptr_t vaddr, uint32_t flags, uint32_t *rflags);

mm_pagedir_t mm_pagedir_alloc(uintptr_t *phys);
void mm_pagedir_free(mm_pagedir_t pd);
void mm_clone(mm_pagedir_t dst, const mm_pagedir_t src);

// void mm_set_kernel(void);
void mm_set(mm_pagedir_t pd);
