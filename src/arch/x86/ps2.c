#include "ps2.h"
#include "irq.h"
#include "sys/debug.h"
#include "sys/task.h"
#include "task.h"

static const char x86_ps2_scan[] = {
    0x00,

    [0x02] = '1',
    [0x03] = '2',
    [0x04] = '3',
    [0x05] = '4',

    [127] = 0
};

void x86_ps2_init(void) {
}

int x86_irq_handler_1(x86_irq_regs_t *regs) {
    /* uint8_t c = */ inb(0x60);
    x86_irq_eoi(1);
    return 0;
}
