#include "sys/assert.h"
#include "dev/initrd.h"
#include "sys/debug.h"
#include "sys/panic.h"
#include "dev/devfs.h"
#include "sys/task.h"
#include "dev/tty.h"
#include "arch/hw.h"
#include "sys/mm.h"
#include "util.h"

void kernel_main(void) {
    // Init basic stuff so we can at least print something
    hw_early_init();
    // Init printing
    debug_init();
    // Proceed on hw-specific details of init
    hw_init();

    // Now the kernel-stuff kicks in
    devfs_init();
    tty_init();
    assert(vfs_mount(NULL, "/dev", vfs_devfs, 0) == 0);
    assert(vfs_mount("/dev/ram0", "/", vfs_initramfs, 0) == 0);
    assert(vfs_mount("/dev/ram0", "/other", vfs_initramfs, 0) == 0);

    vfs_file_t *f;
    ssize_t bread;
    char buf[4096];
    assert(f = vfs_open("/other/bin/test.txt", VFS_FLG_RD));

    while ((bread = vfs_read(f, buf, sizeof(buf), &bread)) > 0) {
        debug("Read block\n");
        debug_dump(buf, bread);
    }

    vfs_close(f);

    // This is where we're ready to accept the first interrupt and start multitasking mode
    irq_enable();
    while (1) {
        __idle();
    }
}
