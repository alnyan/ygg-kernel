#pragma once
#include <stdint.h>
#include "../hw/irq.h"
#include "../regs.h"
#include "../mm.h"
#include "sys/task.h"

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

#define X86_TASK_IDLE           (1 << 1)
// Don't touch task's gp regs
#define X86_TASK_NOGP           (1 << 2)
// Don't touch task's cs:eip info
#define X86_TASK_NOENT          (1 << 3)
// Don't touch task's user stack
#define X86_TASK_NOESP3         (1 << 4)
// Don't allocate signal context
#define X86_TASK_NOSIGCTX       (1 << 5)

struct x86_task_context {
    x86_seg_regs_t segs;
    uint32_t cr3;
    x86_gp_regs_t gp;
    x86_iret_regs_t iret;
};

struct x86_task {
    uint32_t esp0;
    uint32_t ebp0;

    uint32_t esp3_bottom;
    uint32_t esp3_size;

    uint32_t sigeip;
    uint32_t sigesp;

    // TODO: move to ctl
    uint32_t flag;
    uint32_t wait_type;

    mm_space_t pd;
    task_ctl_t *ctl;
    struct x86_task *next;
};

extern struct x86_task *x86_task_current;
extern struct x86_task *x86_task_first;

void x86_task_dump_context(int level, struct x86_task *task);
int x86_task_set_context(struct x86_task *task, uintptr_t entry, void *arg, uint32_t flags);
int x86_task_enter_signal(struct x86_task *task);
int x86_task_exit_signal(struct x86_task *task);

void x86_task_switch(x86_irq_regs_t *regs);
void x86_task_init(void);
