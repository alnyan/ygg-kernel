#pragma once
#include <stdint.h>
#include "irq.h"
#include "regs.h"
#include "mm.h"

// GP regs: 8
// IRET regs: 5
// SEGS regs: 4
// CR3 1
#define X86_TASK_STACK          18
// This allows irq0 (or other switching code) to call something
// And this should be enough
#define X86_TASK_SWITCH_STACK   0
#define X86_TASK_TOTAL_STACK    (X86_TASK_SWITCH_STACK + X86_TASK_STACK)

#define X86_USER_STACK          256

#define X86_TASK_MAX            8

#define X86_TASK_IDLE           (1 << 1)

struct x86_task_ctl {
    uint32_t sleep;
};

struct x86_task {
    uint32_t esp0;
    uint32_t ebp0;
    uint32_t ebp3;
    uint32_t flag;
    struct x86_task_ctl *ctl;
    struct x86_task *next;
};

extern struct x86_task *x86_task_current;

void x86_task_switch(x86_irq_regs_t *regs);
void x86_task_init(void);
