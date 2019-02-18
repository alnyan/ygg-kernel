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

    vfs_dir_t *dir;
    vfs_dirent_t ent;
    assert(dir = vfs_opendir("/"));

    debug("ls \"/\" shows:\n");
    while (vfs_readdir(dir, &ent) == 0) {
        debug("* %s\n", ent.name);
    }

    // This is where we're ready to accept the first interrupt and start multitasking mode
    irq_enable();
    while (1) {
        __idle();
    }
}
