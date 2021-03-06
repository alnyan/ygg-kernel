#include "sys/assert.h"
#include "dev/initrd.h"
#include "sys/debug.h"
#include "sys/panic.h"
#include "dev/devfs.h"
#include "dev/procfs.h"
#include "sys/task.h"
#include "dev/tty.h"
#include "arch/hw.h"
#include "sys/elf.h"
#include "sys/mm.h"
#include "sys/heap.h"
#include "dev/net.h"
#include "util.h"

void kernel_main(void) {
    irq_disable();

    mm_init();
    // Init basic stuff so we can at least print something
    hw_early_init();
    kinfo("Booting %s %s\n", YGG_NAME, YGG_VERSION);
    kinfo("Built for %s, %s\n", YGG_TARGET, YGG_REVDATE);

    // Init printing
    debug_init();
    // Proceed on hw-specific details of init
    hw_init();

    // Now the kernel-stuff kicks in
    devfs_init();
    tty_init();
    // Will create basic device set
    heap_dump();
    devfs_populate();

    assert(vfs_mount(NULL, "/dev", vfs_devfs, 0) == 0);
    assert(vfs_mount("/dev/ram0", "/", vfs_initramfs, 0) == 0);

    // Load network device config
    net_init();
    net_load_config("/etc/network.conf");
    net_dump_ifaces();

    vfs_dirent_t ent;
    vfs_dir_t *dir;
    assert(dir = vfs_opendir("/bin"));

    while (vfs_readdir(dir, &ent) == 0) {
        vfs_dirent_dump(&ent);
    }

    vfs_closedir(dir);

#if defined(ENABLE_TASK)
    assert(task_fexecve("/bin/init", NULL, NULL));
#endif

    // This is where we're ready to accept the first interrupt and start multitasking mode
    net_post_config();

    irq_enable();
    while (1) {
        __idle();
    }
}
