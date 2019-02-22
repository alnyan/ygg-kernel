#pragma once
#include <stdint.h>

#define PCI_ADDRESS(x, y, z)    ((((x) & 0xFF) << 16) | (((y) & 0xFF) << 8) | ((z) & 0xFF))
#define PCI_TRIPLET(a)          (((a) >> 16) & 0xFF), (((a) >> 8) & 0xFF), ((a) & 0xFF)
#define PCI_BUS(a)              (((a) >> 16) & 0xFF)
#define PCI_SLOT(a)             (((a) >> 8) & 0xFF)
#define PCI_FUNC(a)             ((a) & 0xFF)

#define PCI_CONF_VENDOR     0x00
#define PCI_CONF_DEVICE     0x02
#define PCI_CONF_COMMAND    0x04
#define PCI_CONF_STATUS     0x06
#define PCI_CONF_REVCLS     0x08
#define PCI_CONF_HEADER     0x0E

typedef uint32_t pci_addr_t;

uint16_t pci_config_getw(pci_addr_t addr, uint8_t off);
uint32_t pci_config_getl(pci_addr_t addr, uint8_t off);

