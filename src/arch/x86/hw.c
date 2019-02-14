#include "com.h"
#include "gdt.h"
#include "ints.h"
#include "timer.h"
#include "task.h"
#include "def.h"
#include "hw.h"
#include "sys/debug.h"
#include "sys/panic.h"
#include "console.h"
#include "ps2.h"
#include "mm.h"

void hw_early_init(void) {
    com_init(X86_COM0);
    x86_con_init();
}

void hw_init(void) {
    x86_mm_init();

    gdt_init();
    ints_init();

    x86_timer_init(100);
    x86_ps2_init();

    // Add initrd device
    /*struct multiboot_mod_list *mod_list = (struct multiboot_mod_list *) (KERNEL_VIRT_BASE + x86_multiboot_info->mods_addr);*/
    /*debug("Multiboot provided kernel with %d modules\n", x86_multiboot_info->mods_count);*/

    /*if (x86_multiboot_info->mods_count == 0) {*/
        /*panic("Sorry, boot without initrd is not supported yet\n");*/
    /*}*/

    ((uint32_t *) 0x0)[71389] = 1;

    x86_task_init();
}
