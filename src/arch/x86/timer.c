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

uint32_t x86_irq_timer(x86_irq_regs_t *regs) {
    if (regs->iret.cs == 0x08) {
        // Entering usermode
        // Take first task
        uint32_t kesp = x86_task_current();
        debug("Setting kesp to enter US: 0x%x\n", kesp);
        return kesp;
        /*uint32_t *stack = (uint32_t *) kebp;*/
        /*regs->iret.esp = stack[8 + 4];*/
    } else {
        debug("Got kesp 0x%x\n", regs);
        /*// Nothing here yet*/
        /*return regs->gp.oesp + 4;*/
        /*while (1) {}*/
        return (uint32_t) regs;
    }
    /*return ((uint32_t) regs);*/
}
