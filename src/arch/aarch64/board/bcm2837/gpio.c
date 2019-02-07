#include "board.h"

void bcm2837_gpio_init(void) {
    // Disable GPIO push-pull
    bcm2837_gpio->gppud = 0;
    delay(150);
}
