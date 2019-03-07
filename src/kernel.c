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

#include "sys/list.h"

const char *kernel_cmdline = NULL;

LIST(ints, int);
ints_t ints;

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

    // Load network device config
    net_init();
    net_load_config("/etc/network.conf");
    net_dump_ifaces();

    // This is where we're ready to accept the first interrupt and start multitasking mode
    net_post_config();

    list_init(&ints);
    kdebug("Items (0):\n");
    list_foreach(&ints, ints, it) {
        kdebug("* %d\n", it->value);
    }

    for (int i = 0; i < 3; ++i) {
        list_append(&ints, ints, &i);
    }

    kdebug("Items (1):\n");
    list_foreach(&ints, ints, it) {
        kdebug("* %d\n", it->value);
    }

    list_node_free(list_pop_front(&ints));

    kdebug("Items (2):\n");
    list_foreach(&ints, ints, it) {
        kdebug("* %d\n", it->value);
    }

    while (!list_empty(&ints)) {
        NODE(ints) *node = (NODE(ints) *) list_pop_front(&ints);
        list_node_free(node);
    }

    kdebug("Items (3):\n");
    list_foreach(&ints, ints, it) {
        kdebug("* %d\n", it->value);
    }

    while (1);
    irq_enable();
    while (1) {
        __idle();
    }
}
