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

void hw_early_init(void) {
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

    debug("Init module of %uK\n", mod_size / 1024);

    initrd_init(mod_base, mod_size);
}

void hw_init(void) {
    x86_mm_init();

    gdt_init();
    ints_init();

    x86_timer_init(100);
    x86_ps2_init();

    x86_initrd_init();

    x86_task_init();
}