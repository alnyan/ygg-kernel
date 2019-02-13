#pragma once

#define MM_ALIGN_UP(p, a)   (((p) + (a) - 1) & ~((a) - 1))

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
void mm_unmap_cont_region(mm_pagedir_t pd, uintptr_t addr, int count, uint32_t uflags);
void mm_dump_pages(mm_pagedir_t pd);
