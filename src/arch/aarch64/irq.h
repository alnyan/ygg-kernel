#pragma once

#define irq_enable() asm volatile("msr daifclr, #2")
