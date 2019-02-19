#include "ps2.h"
#include "irq.h"
#include "sys/debug.h"
#include "sys/task.h"
#include "dev/tty.h"
#include "task.h"

static const char x86_ps2_scan[] = {
    0x00,

    [0x01] = '\027',

    [0x02] = '1',
             '2',
             '3',
             '4',
             '5',
             '6',
             '7',
             '8',
             '9',
             '0',
             '-',
             '=',
             '\b',
             '\t',

    [0x10] = 'q',
             'w',
             'e',
             'r',
             't',
             'y',
             'u',
             'i',
             'o',
             'p',
             '[',
             ']',
             '\n',

    [0x1e] = 'a',
             's',
             'd',
             'f',
             'g',
             'h',
             'j',
             'k',
             'l',
             ';',
             '\'',
             '`',
             0,
             '\\',

    [0x2c] = 'z',
             'x',
             'c',
             'v',
             'b',
             'n',
             'm',
             ',',
             '.',
             '/',

    [0x39] = ' ',

    [127] = 0
};

void x86_ps2_init(void) {
}

int x86_irq_handler_1(x86_irq_regs_t *regs) {
    uint8_t c = inb(0x60);
    x86_irq_eoi(1);

    if (c < 0x80) {
        tty_type(0, x86_ps2_scan[c]);
    }

    return 0;
}
