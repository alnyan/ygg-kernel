#include "sys/debug.h"
#include "timer.h"
#include "regs.h"
#include "io.h"

#define PIT_BASE_FRQ    1193182
#define PIT_CH0D        0x40
#define PIT_CH1D        0x41
#define PIT_CH2D        0x42
#define PIT_MCMD        0x43

typedef struct {
    x86_seg_regs_t segs;
    x86_gp_regs_t gp;
    x86_iret_regs_t iret;
} x86_irq_regs_t;

void x86_timer_init(uint32_t freq) {
    uint32_t div = PIT_BASE_FRQ / freq;

    if (div > 0xFFFF) {
        debug("Timer divider is >= 65536 (is %u), resetting\n", div);
        div = 0xFFFF;
    }

    // Select CH0, access LO/HI, Square wave generator
    outb(PIT_MCMD, (2 << 4) | (3 << 1));
    io_wait();
    // Write divider parts
    outb(PIT_CH0D, div & 0xFF);
    io_wait();
    outb(PIT_CH0D, (div >> 8) & 0xFF);
}

/*uint32_t x86_irq_timer(x86_irq_regs_t *regs) {*/
    /*if (regs->iret.cs == 0x08) {*/
        /*// Entering usermode*/
        /*// Take first task*/
        /*uint32_t kesp = x86_task_current();*/
        /*debug("Setting kesp to enter US: 0x%x\n", kesp);*/
        /*x86_tss_set(kesp + 17 * 4);*/
        /*return kesp;*/
        /*[>uint32_t *stack = (uint32_t *) kebp;<]*/
        /*[>regs->iret.esp = stack[8 + 4];<]*/
    /*} else {*/
        /*[>uint32_t kesp = x86_task_current();<]*/
        /*[>uint32_t *stack = (uint32_t *) kesp;<]*/
        /*uint32_t old_stack = x86_task_current();*/

        /*#define passert(x) \*/
            /*[>if (!x) { debug("Error: " #x "\n"); while (1); }<]*/

        /*[>debug("Handler pushed this from old task:\n");<]*/
        /*[>for (int i = 0; i < 17; ++i) {<]*/
            /*[>debug("0x%x <esp + %d>: 0x%x\n", old_stack + i * 4, i * 4, ((uint32_t *) old_stack)[i]);<]*/
        /*[>}<]*/

        /*x86_task_switch();*/
        /*uint32_t new_stack = x86_task_current();*/

        /*uint32_t curr_esp;*/
        /*asm volatile ("movl %%esp, %0":"=a"(curr_esp));*/
        /*debug("ESP = 0x%x\n", curr_esp);*/

        /*[>debug("0x%x -> 0x%x\n", old_stack, new_stack);<]*/

        /*[>debug("Handler will pop this from new task:\n");<]*/
        /*[>for (int i = 0; i < 17; ++i) {<]*/
            /*[>debug("0x%x <esp + %d>: 0x%x\n", new_stack + i * 4, i * 4, ((uint32_t *) new_stack)[i]);<]*/
        /*[>}<]*/
        /*[>debug("Stack: 0x%x -> 0x%x\n",<]*/
            /*[>((uint32_t) regs) + 17 * 4,<]*/
            /*[>new_stack + 17 * 4);<]*/

        /*[>for (int i = 0; i < 17; ++i) {<]*/
            /*[>debug("n[-%d] = 0x%x\n", i * 4, ((uint32_t *) new_stack)[i]);<]*/
        /*[>}<]*/

        /*x86_tss_set(new_stack + 17 * 4);*/

        /*return (uint32_t) new_stack;*/
    /*}*/
    /*[>return ((uint32_t) regs);<]*/
/*}*/
