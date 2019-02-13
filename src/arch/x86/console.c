#include "console.h"
#include "sys/mem.h"
#include "arch/hw.h"
#include <stdint.h>

#define X86_CON_BASE    (KERNEL_VIRT_BASE + 0xB8000)

static uint16_t *con_data = (uint16_t *) X86_CON_BASE;
static uint16_t con_cury = 0;
static uint16_t con_curx = 0;

static void x86_con_scroll(void) {
    for (int i = 0; i < 24; ++i) {
        for (int j = 0; j < 80; ++j) {
            con_data[i * 80 + j] = con_data[(i + 1) * 80 + j];
        }
    }
    memset(&con_data[24 * 80], 0, 80 * 2);
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
    } else {
        x86_con_putc('?');
    }
}

void x86_con_init(void) {
    memset(con_data, 0, 80 * 24 * 2);
}
