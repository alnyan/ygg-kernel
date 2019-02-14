#pragma once

// Arch-specific utils
#ifdef ARCH_AARCH64
#define __idle() asm volatile("wfe")
#else
#define __idle() asm volatile("hlt")
#define __idle_halt() asm volatile("cli; hlt")
#endif

