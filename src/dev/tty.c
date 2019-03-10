#include "tty.h"
#include "fs/ioman.h"
#include "sys/debug.h"

#ifdef ARCH_X86
#include "arch/x86/hw/console.h"
#endif

static dev_t tty;
dev_t *dev_tty = &tty;

static int tty_read(dev_t *dev, ioman_op_t *op) {
    dev->pending = op;
    return 0;
}

static ssize_t tty_write(dev_t *dev, const void *buf, uintptr_t pos, size_t count) {
    const char *v = (const char *) buf;
    for (size_t i = 0; i < count; ++i) {
#ifdef ARCH_X86
        x86_con_putc(v[i]);
#endif
    }
    return count;
}

static int tty_would_block(dev_t *dev, size_t count, uintptr_t pos, int dir) {
    return (dir == 0);
}

void tty_init(void) {
    dev_init(&tty, DEV_TYPE_CHR, 0);
    tty.would_block = tty_would_block;
    tty.read = tty_read;
    tty.write = 0;
    tty.write_imm = tty_write;
}

void tty_type(char c) {
    if (tty.pending) {
        if (ioman_buf_write(tty.pending, &c, 1, NULL)) {
            tty.pending = NULL;
        }
    }
}
