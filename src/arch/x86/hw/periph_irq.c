#include "irq.h"
#include "sys/debug.h"
#include "sys/panic.h"

#if defined(ENABLE_RTL8139)
#include "dev/pci/net/rtl8139.h"
#endif

void x86_irq_handler_11(x86_irq_regs_t *regs) {
    // TODO: proper peripheral IRQ routing instead of hardwiring
    kdebug("Peripheral IRQ #11\n");
#if defined(ENABLE_RTL8139)
    if (rtl8139_irq_handler(11) == 0) {
        x86_irq_eoi(11);
        return;
    }
#endif

    panic("Unhandled IRQ!\n");
}
