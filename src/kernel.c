#include "sys/debug.h"
#include "arch/hw.h"
#include "sys/mm.h"
#include "util.h"

void kernel_main(void) {
    hw_early_init();
    debug_init();
    hw_init();

    // Alloc some pages
    debug("Before alloc:\n");
    mm_dump_pages(mm_current);

    // Allocate contiguous region in kernel memory
    uintptr_t v = mm_alloc_kernel_pages(mm_current, 2, MM_AFLG_RW | MM_AFLG_US);

    if (v == MM_NADDR) {
        debug("mm failed to alloc a page\n");
    } else {
        debug("mm allocd %p\n", v);
    }

    // Test some writing
    for (int i = 0; i < (2 * MM_PAGESZ / 64); ++i) {
        ((uint32_t *) v)[i * 16] = i;
    }

    debug("After alloc:\n");
    mm_dump_pages(mm_current);

    // Unmap contiguous region
    mm_unmap_cont_region(mm_current, v, 2, MM_UFLG_PF);

    debug("After unmap:\n");
    mm_dump_pages(mm_current);

    irq_enable();

    while (1) {
        __idle();
    }
}
