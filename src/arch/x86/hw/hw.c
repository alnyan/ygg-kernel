#include "com.h"
#include "gdt.h"
#include "ints.h"
#include "timer.h"
#include "../task/task.h"
#include "../def.h"
#include "hw.h"
#include "sys/debug.h"
#include "sys/panic.h"
#include <stddef.h>
#include "console.h"
#include "ps2.h"
#include "../mm.h"
#include "kernel.h"
#include "sys/string.h"
#include "sys/heap.h"
#include "sys/mm.h"
#include "cpuid.h"
#include "rtc.h"
#include "acpi.h"
#include "hpet.h"
#include "pci.h"
#include "fpu.h"
#include "fs/tarfs.h"
#include "dev/initrd.h"

void (*x86_timer_func) (void);

void hw_early_init(void) {
    // x86_mm_early_init();
    x86_timer_func = NULL;
    com_init(X86_COM0);
    x86_con_init();

    if (x86_multiboot_info->cmdline) {
        kernel_cmdline = (const char *) (x86_multiboot_info->cmdline + KERNEL_VIRT_BASE);

        // Skip filename
        const char *spc = strchrnul(kernel_cmdline, ' ');
        if (*spc) {
            kernel_cmdline = spc + 1;
        }
    }
}

static void x86_initrd_init(void) {
    struct multiboot_mod_list *mod_list = (struct multiboot_mod_list *) (KERNEL_VIRT_BASE + x86_multiboot_info->mods_addr);
    kdebug("Multiboot provided kernel with %d modules\n", x86_multiboot_info->mods_count);

    if (x86_multiboot_info->mods_count != 1) {
        panic("Kernel expected 1 module, but there're %d\n", x86_multiboot_info->mods_count);
    }

    uintptr_t mod_base = KERNEL_VIRT_BASE + mod_list->mod_start;
    size_t mod_size = mod_list->mod_end - mod_list->mod_start;

    kdebug("Init module: %p .. %p\n", mod_base, mod_base + mod_size);
    heap_remove_region(mod_base, mod_size);

    initrd_init(mod_base);
    tarfs_init(dev_initrd);
}

void hw_init(void) {
    // x86_mm_init();
    x86_cpuid_init();
    x86_acpi_init();

    gdt_init();
    ints_init();

    fpu_init();

    // Setup heap
    extern void _kernel_end_virt();
    uintptr_t heap_start = (uintptr_t) _kernel_end_virt + 0x1000;
    uintptr_t heap_end = MM_ALIGN_UP(heap_start, 0x400000);
    kdebug("heap start: %p, heap end: %p\n", heap_start, heap_end);
    heap_add_region(heap_start, heap_end);
    x86_initrd_init();

    if (!hpet_available() || hpet_init() != 0) {
        x86_rtc_init();
        x86_rtc_reload();
        x86_timer_init(100);
    } else {
        x86_timer_func = hpet_timer_func;
    }

    x86_pci_init();

    x86_ps2_init();

#if defined(ENABLE_VESA_FBCON)
    x86_vesa_con_init();
#endif

#if defined(ENABLE_TASK)
    x86_task_init();
#endif
}
