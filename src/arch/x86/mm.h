#pragma once

typedef uintptr_t *mm_pagedir_t;
typedef uintptr_t *mm_pagetab_t;    // Not yet used

void x86_mm_init(void);
