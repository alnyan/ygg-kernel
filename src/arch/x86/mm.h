#pragma once

#define MM_PAGESZ   0x1000

#define MM_NADDR    ((uintptr_t) 0xFFFFFFFF)

typedef int ssize_t;

typedef uint32_t mm_entry_t;
typedef mm_entry_t *mm_pagedir_t;
typedef mm_entry_t *mm_pagetab_t;
typedef mm_pagedir_t mm_space_t;
