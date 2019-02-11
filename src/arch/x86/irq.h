#pragma once

#define irq_enable() asm volatile("sti")
#define irq_disable() asm volatile("cli")

void pic8259_init(void);
