#include "task.h"
#include "sys/debug.h"

// TODO: real allocator
static int s_lastStack;
static char s_stacks[32 * 4 * 16];

static uint32_t s_tasks[2];
static int s_taskIndex = 0;

void task0(void) {
    while (1) {}
}

void task1(void) {
    while (1) {}
}

uint32_t x86_task_create(uint32_t addr) {
    uint32_t ebp0 = (uint32_t) s_stacks + 32 * 4 * (s_lastStack++);
    debug("Allocated task stack: 0x%x\n", ebp0);
    uint32_t *esp0 = (uint32_t *) ebp0;

    *--esp0 = 0;    // EDI
    *--esp0 = 0;    // ESI
    *--esp0 = 0;    // EBP
    *--esp0 = 0;    // oESP
    *--esp0 = 0;    // EBX
    *--esp0 = 0;    // EDX
    *--esp0 = 0;    // ECX
    *--esp0 = 0;    // EAX

    // Task control params
    *--esp0 = 0;    // EIP
    *--esp0 = 0;    // CS
    *--esp0 = 0;    // EFLAGS
    *--esp0 = 0;    // ESP
    *--esp0 = 0;    // SS
}

void x86_task_init(void) {
    debug("Initializing multitasking\n");

    s_tasks[0] = x86_task_create((uint32_t) task0);
    s_tasks[1] = x86_task_create((uint32_t) task1);

    x86_tss_set(s_tasks[0]);
}
