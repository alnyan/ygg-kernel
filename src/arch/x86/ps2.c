#include "ps2.h"
#include "irq.h"
#include "sys/debug.h"
#include "sys/task.h"
#include "task.h"
#include "dev/device.h"

static char x86_ps2_scan[] = {
    0x00,

    [0x02] = '1',
    [0x03] = '2',
    [0x04] = '3',
    [0x05] = '4',

    [127] = 0
};

struct x86_ps2_device {
    dev_t dev;
} x86_ps2_keyboard;

dev_t *dev_keyboard = &x86_ps2_keyboard;

static int x86_ps2_read(dev_t *dev, void *f, void *buf, size_t sz) {
    debug("x86_ps2_read\n");
    io_pending_read_set(f, buf, sz);
    return 0;
}

void x86_ps2_init(void) {
    dev_keyboard->flags = DEV_FLG_READ | DEV_FLG_RDAS;
    dev_keyboard->read = x86_ps2_read;
}

int x86_irq_handler_1(x86_irq_regs_t *regs) {
    uint8_t c = inb(0x60);
    x86_irq_eoi(1);
    vfs_file_t *pend_f;

    if ((pend_f = io_pending_read_first(dev_keyboard, NULL))) {
        if (c < 0x80) {
            c = x86_ps2_scan[c];
            io_pending_read_add(pend_f, &c, 1);
        }
    }

    return 0;
}
