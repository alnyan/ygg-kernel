#include "ps2.h"
#include "irq.h"
#include "sys/debug.h"
#include "sys/panic.h"
#include "sys/ctype.h"
#include "sys/task.h"
#include "ps2cs.h"
#include "sys/assert.h"
#include "dev/tty.h"

#define PS2_FLG_RAW     (1 << 0)
#define PS2_MOD_SHIFT   (1 << 1)
#define PS2_MOD_CAPS    (1 << 2)

#define PS2_LSHIFT_DOWN     0x2A
#define PS2_RSHIFT_DOWN     0x36
#define PS2_LSHIFT_UP       0xAA
#define PS2_RSHIFT_UP       0xB6
#define PS2_CAPS_LOCK_DOWN  0x3A

static uint32_t ps2_flags = 0;

static char ps2_lookup_char(int scan) {
    char c0 = (ps2_flags & PS2_MOD_SHIFT) ? x86_ps2_scan_alt[scan] : x86_ps2_scan[scan];
    return (ps2_flags & PS2_MOD_CAPS) ? togglecase(c0) : c0;
}

void x86_ps2_init(void) {
}

int x86_irq_handler_1(x86_irq_regs_t *regs) {
    uint8_t c = inb(0x60);
    x86_irq_eoi(1);

    if (!(ps2_flags & PS2_FLG_RAW)) {
        switch (c) {
        case PS2_LSHIFT_DOWN:
        case PS2_RSHIFT_DOWN:
            ps2_flags |= PS2_MOD_SHIFT;
            return 0;
        case PS2_LSHIFT_UP:
        case PS2_RSHIFT_UP:
            ps2_flags &= ~PS2_MOD_SHIFT;
            return 0;
        case PS2_CAPS_LOCK_DOWN:
            ps2_flags ^= PS2_MOD_CAPS;
            return 0;
        }
    }

    if (c < 0x80) {
        if (!(ps2_flags & PS2_FLG_RAW)) {
            char r;
            if ((r = ps2_lookup_char(c))) {
                tty_type(r);
            }
        } else {
            panic("Raw mode is not yet implemented\n");
        }
    }

    return 0;
}
