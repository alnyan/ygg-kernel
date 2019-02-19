#pragma once
#include <stdarg.h>
#include <stdint.h>
#include "util.h"

#define PANIC_MSG_INTRO     "--- Kernel panic ---\n"

#define panic(f, ...) panicf("[%s] " f, __func__, ##__VA_ARGS__)

#ifdef ARCH_X86
#define panic_irq(f, r, ...) panicf_irq("[%s] " f, r, __func__, ##__VA_ARGS__)
#define panic_isr(f, r, ...) panicf_isr("[%s] " f, r, __func__, ##__VA_ARGS__)
#include "arch/x86/hw/irq.h"
#include "arch/x86/hw/ints.h"

void panicf_irq(const char *fmt, const x86_irq_regs_t *regs, ...);
void panicf_isr(const char *fmt, const x86_int_regs_t *regs, ...);
#endif

#define panic_hlt() do { __idle_halt(); } while (1)

void panicf(const char *fmt, ...) __attribute__((noreturn));
