#include "tty.h"
#include "sys/dev.h"
#include "sys/mem.h"
#include "sys/debug.h"
#include "sys/panic.h"

#define TTY_COUNT   8

#ifdef ARCH_X86
#include "arch/x86/com.h"
#include "arch/x86/console.h"
#endif

static struct tty {
    dev_t dev;
    uint32_t ttyparam;
} ttys[TTY_COUNT];

static void tty_write_char(struct tty *tty, char c) {
    if (tty->ttyparam == 0) {
#ifdef ARCH_X86
        x86_con_putc(c);
        com_send(X86_COM0, c);
#endif
    } else {
        panic("Write to unsupported tty\n");
    }
}

// TODO: respect write location
static ssize_t tty_write(dev_t *dev, vfs_file_t *f, const void *data, size_t len, uint32_t flags) {
    for (size_t i = 0; i < len; ++i) {
        tty_write_char((struct tty *) dev, ((const char *) data)[i]);
    }
    return len;
}

void tty_init(void) {
    memset(ttys, 0, sizeof(ttys));

    for (int i = 0; i < TTY_COUNT; ++i) {
        dev_init(&ttys[i].dev, DEV_FLG_CHR | DEV_FLG_WR);

        ttys[i].dev.write = tty_write;

        ttys[i].ttyparam = i;
    }
}

dev_t *tty_get(int n) {
    return (dev_t *) &ttys[n];
}
