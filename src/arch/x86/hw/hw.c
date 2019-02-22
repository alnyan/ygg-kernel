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
#include "dev/initrd.h"
#include "sys/heap.h"
#include "cpuid.h"
#include "rtc.h"
#include "acpi.h"
#include "hpet.h"

void (*x86_timer_func) (void);

void hw_early_init(void) {
    x86_timer_func = NULL;
    com_init(X86_COM0);
    x86_con_init();
}

static void x86_initrd_init(void) {
    struct multiboot_mod_list *mod_list = (struct multiboot_mod_list *) (KERNEL_VIRT_BASE + x86_multiboot_info->mods_addr);
    debug("Multiboot provided kernel with %d modules\n", x86_multiboot_info->mods_count);

    if (x86_multiboot_info->mods_count != 1) {
        panic("Kernel expected 1 module, but there're %d\n", x86_multiboot_info->mods_count);
    }

    uintptr_t mod_base = KERNEL_VIRT_BASE + mod_list->mod_start;
    size_t mod_size = mod_list->mod_end - mod_list->mod_start;

    debug("Init module: %p .. %p\n", mod_base, mod_base + mod_size);
    heap_remove_region(mod_base, mod_size);

    initrd_init(mod_base, mod_size);
}

void hw_init(void) {
    x86_mm_init();
    x86_cpuid_init();
    x86_acpi_init();

    gdt_init();
    ints_init();

    if (!hpet_available() || hpet_init() != 0) {
        x86_rtc_init();
        x86_rtc_reload();
        x86_timer_init(100);
    } else {
        x86_timer_func = hpet_timer_func;
    }

    x86_ps2_init();

    x86_initrd_init();

    x86_task_init();
}
