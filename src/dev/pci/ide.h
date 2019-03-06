#pragma once
#include "dev/pci/pci.h"

int pci_ide_init(pci_addr_t addr);
int pci_ide_irq_handler(int n);
