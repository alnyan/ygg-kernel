#pragma once
#if defined(ARCH_AARCH64)
#elif defined(ARCH_X86)
#include "x86/irq.h"
#endif

void hw_init(void);
