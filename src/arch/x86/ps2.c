#include "ps2.h"
#include "irq.h"
#include "sys/debug.h"

void x86_ps2_init(void) {
    debug("TODO: initialize PS/2 controller\n");
}

int x86_irq_handler_1(x86_irq_regs_t *regs) {
    debug("Keyboard interrupt\n");
    char c = inb(0x60);
    x86_irq_eoi(1);

    return 0;
}
