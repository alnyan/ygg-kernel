#include "sys/mm.h"
#include "sys/mem.h"
#include "sys/debug.h"
#include "sys/assert.h"
#include "arch/hw.h"

#include "space.h"
#include "phys.h"
// Kernel space internal subdivision
//  16MiB - kernel itself
//  16MiB - paging structure pool

mm_space_t mm_kernel;

void mm_set(mm_space_t pd) {
    uint32_t cr3 = pd[0];
    asm volatile ("mov %0, %%cr3"::"a"(cr3));
}

void mm_init(void) {
    x86_mm_phys_init();
    x86_mm_pool_init();

    // Set mm_kernel to page directory from boot stage
    extern uint32_t boot_page_directory[1024];
    mm_kernel = boot_page_directory;

    // Attach table_info structure to mm_kernel
    uint32_t *kernel_table_info = (uint32_t *) x86_page_pool_allocate(NULL);
    assert((uintptr_t) kernel_table_info != MM_NADDR);
    memsetl(kernel_table_info, 0, 1024);
    mm_kernel[1023] = (uint32_t) kernel_table_info;
    mm_kernel[0] = (uint32_t) mm_kernel - KERNEL_VIRT_BASE;

    mm_dump_map(DEBUG_DEFAULT, mm_kernel);
}
