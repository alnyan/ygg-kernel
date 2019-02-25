#pragma once
#include <stddef.h>
#include "dev/pci/pci.h"
#include "arch/x86/hw/irq.h"

#define RTL8139_PCI_IOAR    0x10
#define RTL8139_PCI_MEMAR   0x14

int rtl8139_send(const void *buf, size_t size);

void rtl8139_test(void);
int rtl8139_init(pci_addr_t addr);
int rtl8139_irq_handler(int irq_no);
