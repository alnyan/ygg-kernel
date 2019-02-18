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

    // This is where we're ready to accept the first interrupt and start multitasking mode
    irq_enable();
    while (1) {
        __idle();
    }
}
