#include "board.h"
#include "sys/debug.h"

void aarch64_irq_handler(void) {
    // If it's timer, clr
    if (bcm2835_irq_regs->irq_basic_pending & 1) {
        debug("Timer tick\n");
        bcm2835_irq_regs->irq_basic_pending &= ~1;
        bcm283xsp804->irq_clr = 1;
        return;
    }
}
