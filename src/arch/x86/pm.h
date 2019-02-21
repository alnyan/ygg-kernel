#pragma once
#include <stdint.h>
#include <stddef.h>

void x86_pm_init(void);
void x86_mm_claim_page(uintptr_t page);
