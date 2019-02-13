#include "task.h"
#include "arch/hw.h"
#include "sys/debug.h"

// GP regs: 8
// IRET regs: 5
// SEGS regs: 4
// CR3 1
#define X86_TASK_STACK          18
// This allows irq0 (or other switching code) to call something
// And this should be enough
#define X86_TASK_SWITCH_STACK   0

#define X86_TASK_TOTAL_STACK    (X86_TASK_SWITCH_STACK + X86_TASK_STACK)

// TODO: real allocator
static int s_lastStack = 0;
static uint32_t s_pagedirs[1024 * 4] __attribute__((aligned(4096)));
static uint32_t s_stacks[X86_TASK_TOTAL_STACK * 4];
static uint32_t s_userStacks[256 * 4];

uint32_t x86_tasks[4];
int x86_task_index = 0;

void task0(void *arg) {
    uint16_t *myptr = (uint16_t *) arg;
    *myptr = 0x0200 | 'A';

    while (1) {
        for (int i = 0; i < 0x0400000; ++i) {
        }

        *myptr ^= 0x2200 | 'A';
    }
}

uint32_t x86_task_create(void (*addr)(void *), void *arg) {
    uint32_t stackIndex = s_lastStack++;
    uint32_t ebp0 = (uint32_t) &s_stacks[stackIndex * X86_TASK_TOTAL_STACK + X86_TASK_TOTAL_STACK];
    uint32_t ebp3 = (uint32_t) &s_userStacks[stackIndex * 256 + 256];
    uint32_t cr3 = (uint32_t) &s_pagedirs[stackIndex * 1024];
    debug("Allocated task stack: 0x%x\n", ebp0);
    uint32_t *esp0 = (uint32_t *) ebp0;
    uint32_t *esp3 = (uint32_t *) ebp3;

    memset((void *) cr3, 0, 1024 * 4);
    ((uint32_t *) cr3)[768] = 0x87;     // TODO: replace with actual kernel pagedir cloning

    cr3 -= KERNEL_VIRT_BASE;

    // Push function argument
    *--esp3 = (uint32_t) arg;
    // Push some random return address, as return from userspace thread is NYI
    *--esp3 = 0x12345678;

    *--esp0 = 0x23; // SS
    *--esp0 = (uint32_t) esp3; // ESP
    *--esp0 = 0x248;    // EFLAGS
    *--esp0 = 0x1B; // CS
    *--esp0 = (uint32_t) addr; // EIP
    // Task control params

    *--esp0 = 0;    // EAX
    *--esp0 = 0;    // ECX
    *--esp0 = 0;    // EDX
    *--esp0 = 0;    // EBX
    *--esp0 = 0;    // oESP
    *--esp0 = 0;    // EBP
    *--esp0 = 0;    // ESI
    *--esp0 = 0;    // EDI

    *--esp0 = cr3;   // XXX: actually should be a real cr3

    *--esp0 = 0x23;
    *--esp0 = 0x23;
    *--esp0 = 0x23;
    *--esp0 = 0x23;

    return (uint32_t) esp0;
}

void x86_task_init(void) {
    debug("Initializing multitasking\n");

    s_lastStack = 0;

    for (int i = 0; i < 4; ++i) {
        x86_tasks[i] = x86_task_create(task0, (void *) (0xC00B8000 + i * 2));
    }
}

void task_start(void) {

}
