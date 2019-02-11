#include "com.h"
#include "ints.h"

void hw_init(void) {
    com_init(X86_COM0);

    ints_init();
}
