#include "sys/assert.h"
#include "sys/debug.h"
#include "sys/panic.h"
#include "sys/task.h"
#include "arch/hw.h"
#include "sys/elf.h"
#include "sys/mm.h"
#include "fs/ioman.h"
#include "sys/heap.h"
#include "dev/net.h"
#include "dev/tty.h"
#include "util.h"

const char *kernel_cmdline = NULL;

void kernel_main(void) {
    irq_disable();

    mm_init();
    // Init basic stuff so we can at least print something
    hw_early_init();
    kinfo("Booting %s %s\n", YGG_NAME, YGG_VERSION);
    kinfo("Built for %s, %s\n", YGG_TARGET, YGG_REVDATE);
    kinfo("Kernel command line: %s\n", kernel_cmdline);

    // Init printing
    debug_init();
    // Proceed on hw-specific details of init
    hw_init();
    tty_init();

    // Load network device config
    net_init();
    net_load_config("/etc/network.conf");
    net_dump_ifaces();

    ioman_init();

    // This is where we're ready to accept the first interrupt and start multitasking mode
    net_post_config();

    ioman_start_task();
    irq_enable();

    while (1) {
        __idle();
    }
}
