#include "rtc.h"
#include "sys/debug.h"
#include "io.h"

#define RTC_FREQ    1024

static struct {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t weekday;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} rtc_cmos_state;

static uint8_t cmos_read(int reg) {
    outb(0x70, (1 << 7) | reg);
    return inb(0x71);
}

static volatile uint32_t rtc_counter = 0;
static uint16_t rtc_century_addr = 0;

static void x86_rtc_fetch(void) {
    rtc_cmos_state.sec = cmos_read(0x00);
    rtc_cmos_state.min = cmos_read(0x02);
    rtc_cmos_state.hour = cmos_read(0x04);

    rtc_cmos_state.day = cmos_read(0x07);
    rtc_cmos_state.month = cmos_read(0x08);
    if (rtc_century_addr == 0) {
        rtc_cmos_state.year = cmos_read(0x09) + 2000;
    } else {
        rtc_cmos_state.year = cmos_read(rtc_century_addr) * 100 + cmos_read(0x09);
    }
}

void x86_rtc_set_century_addr(uint16_t addr) {
    rtc_century_addr = addr;
}

void x86_rtc_reload(void) {
    kdebug("Reloading RTC registers\n");
    x86_rtc_fetch();
    kdebug("RTC datetime is: %04u/%02u/%02u, %02u:%02u:%02u\n",
        rtc_cmos_state.year,
        rtc_cmos_state.month,
        rtc_cmos_state.day,
        rtc_cmos_state.hour,
        rtc_cmos_state.min,
        rtc_cmos_state.sec);
}

void x86_rtc_init(void) {
    kdebug("Initializing RTC\n");
    outb(0x70, 0x8B);
    uint8_t prev = inb(0x71);
    outb(0x70, 0x8B);
    outb(0x71, prev | 0x44);
}

void x86_irq_handler_8(x86_irq_regs_t *regs) {
    outb(0x70, 0x0C);
    inb(0x71);

    x86_irq_eoi(8);

    ++rtc_counter;
    if (rtc_counter == RTC_FREQ) {
        rtc_counter = 0;
        // TODO: update system time
    }
}
