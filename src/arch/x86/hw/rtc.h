#pragma once
#include "irq.h"

void x86_rtc_set_century_addr(uint16_t off);
void x86_rtc_interrupt(x86_irq_regs_t *regs);
void x86_rtc_reload(void);
void x86_rtc_init(void);
