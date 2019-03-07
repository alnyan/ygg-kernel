#pragma once

#if defined(ARCH_X86)
#include "arch/x86/thread/spin.h"
#else
#error "Not implemented"
#endif

void spin_lock(spin_t *s);
void spin_unlock(spin_t *s);
