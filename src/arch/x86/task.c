#include "task.h"
#include "arch/hw.h"
#include <stddef.h>
#include "sys/task.h"
#include "sys/debug.h"
#include "sys/mem.h"

// GP regs: 8
// IRET regs: 5
// SEGS regs: 4
// CR3 1
#define X86_TASK_STACK          18
// This allows irq0 (or other switching code) to call something
// And this should be enough
#define X86_TASK_SWITCH_STACK   0

#define X86_USER_STACK          256

#define X86_TASK_MAX            8

#define X86_TASK_TOTAL_STACK    (X86_TASK_SWITCH_STACK + X86_TASK_STACK)

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

// TODO: real allocator
static int s_lastStack = 0;
static int s_lastThread = 0;
static uint32_t s_pagedirs[1024 * X86_TASK_MAX] __attribute__((aligned(4096)));
static uint32_t s_stacks[X86_TASK_TOTAL_STACK * X86_TASK_MAX];
static uint32_t s_userStacks[X86_USER_STACK * X86_TASK_MAX];
static struct x86_task s_taskStructs[sizeof(struct x86_task) * X86_TASK_MAX];
static struct x86_task_ctl s_taskCtls[sizeof(struct x86_task_ctl) * X86_TASK_MAX];

struct x86_task *x86_task_current = NULL;
struct x86_task *x86_task_first = NULL;
static struct x86_task *x86_task_idle = NULL;

extern void x86_task_idle_func(void *arg);

void task0(void *arg) {
    uint16_t *myptr = (uint16_t *) arg;
    *myptr = 0x0200 | 'A';

    while (1) {
        *myptr ^= 0x2200 | 'A';
    }
}

struct x86_task *x86_task_alloc(void) {
    struct x86_task *t = &s_taskStructs[s_lastThread++];
    t->ctl = &s_taskCtls[s_lastThread - 1];
    return t;
}

void x86_task_setup(struct x86_task *t, void (*entry)(void *), void *arg, int flag) {
    uint32_t stackIndex = s_lastStack++;

    // Create kernel-space stack for state storage
    t->ebp0 = (uint32_t) &s_stacks[stackIndex * X86_TASK_TOTAL_STACK + X86_TASK_TOTAL_STACK];
    // Create user-space stack
    t->ebp3 = (uint32_t) &s_userStacks[stackIndex * X86_USER_STACK + X86_USER_STACK];
    // Create a page dir
    uint32_t cr3 = (uint32_t) &s_pagedirs[stackIndex * 1024];
    memset((void *) cr3, 0, 4096);
    ((uint32_t *) cr3)[768] = 0x87;
    cr3 -= KERNEL_VIRT_BASE;

    uint32_t *esp0 = (uint32_t *) t->ebp0;
    uint32_t *esp3 = (uint32_t *) t->ebp3;

    *--esp3 = (uint32_t) arg;   // Push thread arg
    *--esp3 = 0x12345678;       // Push some funny return address

    *--esp0 = 0x23;             // SS
    *--esp0 = (uint32_t) esp3;  // ESP
    *--esp0 = 0x248;            // EFLAGS
    if (flag & 1) {
        *--esp0 = 0x08;             // CS
    } else {
        *--esp0 = 0x1B;             // CS
    }
    *--esp0 = (uint32_t) entry; // EIP

    // Push GP regs
    for (int i = 0; i < 8; ++i) {
        *--esp0 = 0;
    }

    *--esp0 = cr3;              // CR3

    // Push segs
    for (int i = 0; i < 4; ++i) {
        *--esp0 = 0x23;
    }

    t->esp0 = (uint32_t) esp0;
}

void x86_task_init(void) {
    debug("Initializing multitasking\n");

    s_lastStack = 0;

    struct x86_task *prev_task = NULL;
    struct x86_task *task;

    // Create idle task (TODO: make it kernel space, so it can HLT)
    task = x86_task_alloc();
    task->next = NULL;
    task->flag = 0;
    x86_task_setup(task, x86_task_idle_func, NULL, 1);

    x86_task_idle = task;
    x86_task_current = task;
    x86_task_first = task;

    prev_task = task;

    for (int i = 0; i < 7; ++i) {
        task = x86_task_alloc();

        task->next = NULL;
        task->flag = 0;

        x86_task_setup(task, task0, (void *) (0xC00B8000 + i * 2), 0);

        prev_task->next = task;

        prev_task = task;
    }
}

void x86_task_switch(x86_irq_regs_t *regs) {
    if (regs->iret.cs == 0x08) {
        return;
    }

    // Update tasks' flags and ctl
    for (struct x86_task *t = x86_task_first; t; t = t->next) {
        if (t == x86_task_idle) {
            continue;
        }
        // Decrease sleep counters
        if (t->flag & TASK_FLG_WAIT) {
            if (!(--t->ctl->sleep)) {
                t->flag &= ~TASK_FLG_WAIT;
            }
        }
    }

    struct x86_task *from = x86_task_current;

    x86_task_current->flag &= ~TASK_FLG_RUNNING;

    while (1) {
        if (!(x86_task_current = x86_task_current->next)) {
            x86_task_current = x86_task_first;
        }

        if (x86_task_current->flag & TASK_FLG_WAIT) {
            // If a loop was done and we couldn't find any task, just use the previous one
            if (x86_task_current == from) {
                break;
            }
            continue;
        }

        break;
    }

    if (x86_task_current == from && (x86_task_current->flag & TASK_FLG_WAIT)) {
        // If we couldn't find any non-waiting task
        x86_task_current = x86_task_idle;
        x86_task_current->flag |= TASK_FLG_RUNNING;
        return;
    }

    if (from != x86_task_idle) {
        from->ctl->sleep = 100;
        from->flag |= TASK_FLG_WAIT;
    }

    x86_task_current->flag |= TASK_FLG_RUNNING;
}
