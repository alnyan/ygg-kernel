#include "com.h"
#include "io.h"

void com_init(uint16_t port) {
    // Do nothing yet
}

void com_send(uint16_t port, char v) {
    while (!(inb(port + 5) & 0x20)) {}
	outb(port, v);
}
