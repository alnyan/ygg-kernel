#include "console.h"
#include "sys/mem.h"
#include "arch/hw.h"
#include <stdint.h>

#define X86_CON_BASE    (KERNEL_VIRT_BASE + 0xB8000)

static uint16_t *con_data = (uint16_t *) X86_CON_BASE;
static uint16_t con_cury = 0;
static uint16_t con_curx = 0;

static void x86_con_cursor(uint16_t row, uint16_t col) {
    uint16_t pos = row * 80 + col;

    outb(0x03D4, 0x0F);
	outb(0x03D5, (uint8_t) (pos & 0xFF));
	outb(0x03D4, 0x0E);
	outb(0x03D5, (uint8_t) ((pos >> 8) & 0xFF));
}

static void x86_con_scroll(void) {
    for (int i = 0; i < 24; ++i) {
        for (int j = 0; j < 80; ++j) {
            con_data[i * 80 + j] = con_data[(i + 1) * 80 + j];
        }
    }
    memsetw(&con_data[24 * 80], 0x0700, 80);
}

void x86_con_putc(char c) {
    if (c >= ' ') {
        if (con_curx == 79) {
            con_curx = 0;
            ++con_cury;
        }

        if (con_cury == 25) {
            con_cury = 24;
            x86_con_scroll();
        }

        con_data[con_cury * 80 + con_curx] = 0x0700 | c;
        ++con_curx;
    } else if (c == '\n') {
        con_curx = 0;
        ++con_cury;

        if (con_cury == 24) {
            con_cury = 23;
            x86_con_scroll();
        }
    } else if (c == '\b') {
        // XXX: should cursor move back one line if at beginning?
        if (con_curx) {
            con_data[con_cury * 80 + (--con_curx)] = 0x0700;
        }
    } else {
        x86_con_putc('?');
    }

    x86_con_cursor(con_cury, con_curx);
}

void x86_con_init(void) {
    memsetw(con_data, 0x0700, 80 * 24);
}
