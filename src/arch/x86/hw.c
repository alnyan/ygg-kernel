#include "com.h"
#include "gdt.h"
#include "ints.h"
#include "timer.h"
#include "task.h"
#include "ps2.h"
#include "mm.h"

void hw_early_init(void) {
    com_init(X86_COM0);
}

void hw_init(void) {
    x86_mm_init();

    gdt_init();
    ints_init();

    x86_timer_init(100);
    x86_ps2_init();

    x86_task_init();
}
