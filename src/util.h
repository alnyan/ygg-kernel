#pragma once

// Arch-specific utils
#if defined(ARCH_AARCH64)
#include "arch/aarch64/irq.h"
#define __idle() asm volatile("wfe")
#define __idle_halt() asm volatile("msr daifset, #2")
#elif defined(ARCH_X86)
#define __idle() asm volatile("hlt")
#define __idle_halt() asm volatile("cli; hlt")
#endif

