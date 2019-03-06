#include "irq.h"
#include "sys/debug.h"
#include "sys/panic.h"

#if defined(ENABLE_RTL8139)
#include "dev/pci/net/rtl8139.h"
#endif
#include "dev/pci/ide.h"

void x86_irq_handler_11(x86_irq_regs_t *regs) {
    // TODO: proper peripheral IRQ routing instead of hardwiring
    kdebug("Peripheral IRQ #11: Free\n");
#if defined(ENABLE_RTL8139)
    if (rtl8139_irq_handler(11) == 0) {
        x86_irq_eoi(11);
        return;
    }
#endif

    panic("Unhandled IRQ!\n");
}

void x86_irq_handler_14(x86_irq_regs_t *regs) {
    kdebug("Peripheral IRQ #14: ATA\n");

    if (pci_ide_irq_handler(14) == 0) {
        x86_irq_eoi(14);
        return;
    }
}
