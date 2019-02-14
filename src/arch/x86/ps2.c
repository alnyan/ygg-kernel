#include "ps2.h"
#include "irq.h"
#include "sys/debug.h"
#include "sys/task.h"
#include "task.h"

void x86_ps2_init(void) {
    debug("TODO: initialize PS/2 controller\n");
}

int x86_irq_handler_1(x86_irq_regs_t *regs) {
    uint8_t c = inb(0x60);
    x86_irq_eoi(1);

    // Notify all tasks input is ready
    for (struct x86_task *t = x86_task_first; t; t = t->next) {
        if (c < 0x80) {
            if ((t->flag & TASK_FLG_BUSY) && t->ctl->readc) {
                // We've read one char
                --t->ctl->readc;

                // Read everything - task isn't busy
                if (!t->ctl->readc) {
                    t->flag &= ~TASK_FLG_BUSY;
                }
            }
        }
    }

    return 0;
}
