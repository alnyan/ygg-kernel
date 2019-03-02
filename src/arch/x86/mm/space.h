#pragma once

uintptr_t x86_page_pool_allocate(uintptr_t *phys);
void x86_page_pool_free(uintptr_t addr);

void x86_mm_pool_init(void);
