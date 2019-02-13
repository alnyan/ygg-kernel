#include "task.h"
#include "arch/hw.h"
#include <stddef.h>
#include "sys/debug.h"

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

struct x86_task {
    uint32_t esp0;
    uint32_t ebp0;
    uint32_t ebp3;
    struct x86_task *next;
};

// TODO: real allocator
static int s_lastStack = 0;
static int s_lastThread = 0;
static uint32_t s_pagedirs[1024 * X86_TASK_MAX] __attribute__((aligned(4096)));
static uint32_t s_stacks[X86_TASK_TOTAL_STACK * X86_TASK_MAX];
static uint32_t s_userStacks[X86_USER_STACK * X86_TASK_MAX];
static struct x86_task s_taskStructs[sizeof(struct x86_task) * X86_TASK_MAX];

struct x86_task *x86_task_current = NULL;
struct x86_task *x86_task_first = NULL;

/*uint32_t x86_tasks[4];*/
/*int x86_task_index = 0;*/

void task0(void *arg) {
    uint16_t *myptr = (uint16_t *) arg;
    *myptr = 0x0200 | 'A';

    while (1) {
        for (int i = 0; i < 0x0400000; ++i) {
        }

        *myptr ^= 0x2200 | 'A';
    }
}

struct x86_task *x86_task_alloc(void) {
    return &s_taskStructs[s_lastThread++];
}

void x86_task_setup(struct x86_task *t, void (*entry)(void *), void *arg) {
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
    *--esp0 = 0x1B;             // CS
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

    for (int i = 0; i < 8; ++i) {
        struct x86_task *task = x86_task_alloc();

        task->next = NULL;

        if (!x86_task_first) {
            debug("0x%x is the first\n", task);
            x86_task_first = task;
            x86_task_current = task;
        }

        x86_task_setup(task, task0, (void *) (0xC00B8000 + i * 2));

        if (prev_task) {
            prev_task->next = task;
            debug("0x%x's next is 0x%x\n", prev_task, task);
        }

        prev_task = task;
    }
}

void task_start(void) {

}
