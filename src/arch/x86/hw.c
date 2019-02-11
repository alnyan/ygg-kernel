#include "com.h"
#include "gdt.h"
#include "ints.h"
#include "timer.h"
#include "mm.h"

void hw_init(void) {
    com_init(X86_COM0);

    x86_mm_init();

    gdt_init();
    ints_init();

    x86_timer_init(100);
}
