#pragma once

// Compiler-specific utils
// TODO: move to somewhere like "util/gcc.h"
#define offsetof(t, m) __builtin_offsetof(t, m)

// Arch-specific utils
#ifdef ARCH_AARCH64
#define __idle() asm volatile("wfe")
#else
#define __idle() asm volatile("hlt")
#define __idle_halt() asm volatile("cli; hlt")
#endif

