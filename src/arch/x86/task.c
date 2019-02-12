#include "task.h"
#include "sys/debug.h"

// TODO: real allocator
static int s_lastStack = 0;
static uint32_t s_stacks[32 * 16];
static uint32_t s_userStacks[256 * 16];

static uint32_t s_tasks[2];
static int s_taskIndex = 0;


void task0(void) {
    uint16_t *myptr = (uint16_t *) 0xC00B8000;
    *myptr = 0;

    while (1) {
        for (int i = 0; i < 0x04000000; ++i) {
        }

        *myptr ^= 0x0700 | 'A';
    }
}

void task1(void) {
    uint16_t *myptr = (uint16_t *) 0xC00B8002;
    *myptr = 0;

    while (1) {
        for (int i = 0; i < 0x04000000; ++i) {
        }

        *myptr ^= 0x0800 | 'B';
    }
}

uint32_t x86_task_current(void) {
    return s_tasks[s_taskIndex];
}

uint32_t x86_task_create(uint32_t addr) {
    uint32_t stackIndex = s_lastStack++;
    uint32_t ebp0 = (uint32_t) &s_stacks[stackIndex * 32 + 32];
    uint32_t ebp3 = (uint32_t) &s_userStacks[stackIndex * 256 + 256];
    debug("Allocated task stack: 0x%x\n", ebp0);
    uint32_t *esp0 = (uint32_t *) ebp0;

    *--esp0 = 0x23; // SS
    *--esp0 = ebp3; // ESP
    *--esp0 = 0x248;    // EFLAGS
    *--esp0 = 0x1B; // CS
    *--esp0 = addr; // EIP
    // Task control params

    *--esp0 = 0;    // EAX
    *--esp0 = 0;    // ECX
    *--esp0 = 0;    // EDX
    *--esp0 = 0;    // EBX
    *--esp0 = 0;    // oESP
    *--esp0 = 0;    // EBP
    *--esp0 = 0;    // ESI
    *--esp0 = 0;    // EDI

    *--esp0 = 0x23;
    *--esp0 = 0x23;
    *--esp0 = 0x23;
    *--esp0 = 0x23;

    return (uint32_t) esp0;
}

void x86_task_init(void) {
    debug("Initializing multitasking\n");

    s_lastStack = 0;

    s_tasks[0] = x86_task_create((uint32_t) task0);
    s_tasks[1] = x86_task_create((uint32_t) task1);

    x86_tss_set(s_tasks[0]);
}

void task_start(void) {

}
