#include "sys/debug.h"
#include "timer.h"
#include "io.h"

#define PIT_BASE_FRQ    1193182
#define PIT_CH0D        0x40
#define PIT_CH1D        0x41
#define PIT_CH2D        0x42
#define PIT_MCMD        0x43

void x86_timer_init(uint32_t freq) {
    uint32_t div = PIT_BASE_FRQ / freq;

    if (div > 0xFFFF) {
        debug("Timer divider is >= 65536 (is %u), resetting\n", div);
        div = 0xFFFF;
    }

    // Select CH0, access LO/HI, Square wave generator
    outb(PIT_MCMD, (2 << 4) | (3 << 1));
    io_wait();
    // Write divider parts
    outb(PIT_CH0D, div & 0xFF);
    io_wait();
    outb(PIT_CH0D, (div >> 8) & 0xFF);
}
