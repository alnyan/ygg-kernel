#include "sys/debug.h"
#include "arch/hw.h"
#include "sys/panic.h"
#include "sys/heap.h"
#include "sys/mm.h"
#include "util.h"

void kernel_main(void) {
    hw_early_init();
    debug_init();
    hw_init();

    heap_dump();

    irq_enable();

    while (1) {
        __idle();
    }
}
