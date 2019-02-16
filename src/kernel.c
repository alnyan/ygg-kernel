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

    struct heap_stat st;
    heap_stat(&st);
    debug("T = %u\n", st.total);
    heap_dump();

    void *p0 = heap_alloc(512);
    void *p1 = heap_alloc(512);
    void *p2 = heap_alloc(512);
    heap_free(p1);
    heap_dump();

    p0 = heap_realloc(p0, 512 + 511);
    debug("After realloc\n");
    heap_dump();

    heap_free(p0);
    heap_free(p2);

    heap_stat(&st);
    debug("T = %u\n", st.total);
    heap_dump();

    irq_enable();

    while (1) {
        __idle();
    }
}
