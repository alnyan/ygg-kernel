#include "sys/assert.h"
#include "sys/debug.h"
#include "sys/panic.h"
#include "sys/task.h"
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

    // Load network device config
    net_init();
    net_load_config("/etc/network.conf");
    net_dump_ifaces();

    // This is where we're ready to accept the first interrupt and start multitasking mode
    net_post_config();

    irq_enable();
    while (1) {
        __idle();
    }
}
