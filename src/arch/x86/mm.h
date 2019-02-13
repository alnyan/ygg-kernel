#pragma once

#define MM_NADDR    ((uintptr_t) 0xFFFFFFFF)
#define MM_PAGESZ   0x400000

typedef uintptr_t *mm_pagedir_t;
typedef uintptr_t *mm_pagetab_t;    // Not yet used

extern mm_pagedir_t mm_kernel;
extern mm_pagedir_t mm_current;

void x86_mm_init(void);
