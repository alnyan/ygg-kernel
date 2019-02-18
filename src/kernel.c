#include "sys/assert.h"
#include "sys/debug.h"
#include "sys/panic.h"
#include "dev/devfs.h"
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
    if (vfs_mount(NULL, "/dev", vfs_devfs, 0) != 0) {
        panic("Failed to mount devfs\n");
    }

    vfs_file_t *f;
    assert(f = vfs_open("/dev/zero", VFS_FLG_RD));

    char buf[16];
    for (char c = 1; c < sizeof(buf); ++c) {
        buf[c - 1] = c * 13;
    }
    debug_dump(buf, sizeof(buf));

    assert(vfs_read(f, buf, sizeof(buf)) == 16);

    debug_dump(buf, sizeof(buf));

    vfs_close(f);

    // This is where we're ready to accept the first interrupt and start multitasking mode
    irq_enable();
    while (1) {
        __idle();
    }
}
