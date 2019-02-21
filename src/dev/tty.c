#include "tty.h"
#include "sys/dev.h"
#include "sys/mem.h"
#include "sys/assert.h"
#include "sys/debug.h"
#include "sys/panic.h"
#include "sys/vfs.h"

#define TTY_COUNT   8

#ifdef ARCH_X86
#include "arch/x86/hw/com.h"
#include "arch/x86/hw/console.h"
#endif

static struct tty {
    dev_t dev;
    uint32_t ttyparam;
    vfs_file_t *pending;
} ttys[TTY_COUNT];

static void tty_write_char(struct tty *tty, char c) {
    switch (tty->ttyparam) {
#ifdef ARCH_X86
    // ttyS0
    case TTY_FLG_SER:
        com_send(X86_COM0, c);
        break;
    // tty0
    case 0:
        x86_con_putc(c);
        break;
#endif
    }
}

static ssize_t tty_write(dev_t *dev, vfs_file_t *f, const void *data, size_t len, uint32_t flags) {
    for (size_t i = 0; i < len; ++i) {
        tty_write_char((struct tty *) dev, ((const char *) data)[i]);
    }
    return len;
}

static int tty_read(dev_t *dev, vfs_file_t *f, void *data, size_t len, uint32_t flags) {
    struct tty *tty = (struct tty *) dev;

    if (tty->pending) {
        panic("TTY is already locked by some other operation\n");
    }

    tty->pending = f;

    return VFS_READ_ASYNC;
}

void tty_init(void) {
    memset(ttys, 0, sizeof(ttys));

#ifdef ARCH_X86
    for (int i = 0; i < 2; ++i) {
        dev_init(&ttys[i].dev, DEV_FLG_CHR | DEV_FLG_WR);
        ttys[i].dev.write = tty_write;
        ttys[i].dev.read = tty_read;
    }

    ttys[0].ttyparam = 0;
    ttys[1].ttyparam = 0xF0000000;
#endif
}

dev_t *tty_get(int n) {
#ifdef ARCH_X86
    if (n == 0xF0000000) {
        return (dev_t *) &ttys[1];
    }
    if (n == 0) {
        return (dev_t *) ttys;
    }
#endif
    return NULL;
}

void tty_type(int n, char c) {
    assert(n == 0);
    struct tty *tty = (struct tty *) tty_get(n);

    // I know this is not how that actually should work,
    // but for not just let it work without buffering
    if (tty->pending) {
        assert(tty->pending->op_buf);
        assert(tty->pending->op_res);

        if (vfs_send_read_res(tty->pending, &c, 1) == 1) {
            tty->pending = NULL;
        }
    }
}
