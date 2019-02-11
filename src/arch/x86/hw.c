#include "com.h"
#include "gdt.h"
#include "ints.h"

void hw_init(void) {
    com_init(X86_COM0);

    gdt_init();
    ints_init();

    int a = 1 / 0;
}
