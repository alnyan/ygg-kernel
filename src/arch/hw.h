#pragma once
#if defined(ARCH_AARCH64)   // ARCH_X86

#define KERNEL_VIRT_BASE        0xC0000000

#elif defined(ARCH_X86)     // !ARCH_AARCH64 && ARCH_X86

#define KERNEL_VIRT_BASE        0xC0000000
#include "x86/irq.h"

#endif                      // !ARCH_X86

void hw_init(void);
