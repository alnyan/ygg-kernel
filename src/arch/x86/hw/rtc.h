#pragma once
#include "irq.h"

void x86_rtc_interrupt(x86_irq_regs_t *regs);
void x86_rtc_init(void);
