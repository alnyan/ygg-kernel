#include "pci.h"
#include <stdint.h>
#include "sys/debug.h"
#include "pciclass.h"
#include "io.h"

#define PCI_CONFIG_CMD  0x0CF8
#define PCI_CONFIG_DAT  0x0CFC

typedef uint32_t pci_addr_t;

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

static uint16_t pci_config_getw(pci_addr_t addr, uint8_t off) {
    uint32_t address;
    address = (uint32_t)((((uint32_t) PCI_BUS(addr)) << 16) | (((uint32_t) PCI_SLOT(addr)) << 11) |
              (((uint32_t) PCI_FUNC(addr)) << 8) | (off & 0xfc) | ((uint32_t) 0x80000000));
    outl(PCI_CONFIG_CMD, address);
    return (uint16_t) ((inl(0xCFC) >> ((off & 2) * 8)) & 0xFFFF);
}

static uint32_t pci_config_getl(pci_addr_t addr, uint8_t off) {
    return pci_config_getw(addr, off) | ((uint32_t) pci_config_getw(addr, off + 2) << 16);
}

static void pci_scan_func(pci_addr_t addr) {
    uint32_t class_code = (pci_config_getl(addr, PCI_CONF_REVCLS) >> 8) & 0xFFFFFF;
    uint8_t class = (class_code >> 16) & 0xFF;
    uint8_t subclass = (class_code >> 8) & 0xFF;

    if (class == 0x06 && subclass == 0x04) {
        debug("TODO: scan secondary bus\n");
    } else {
        debug("PCI %d:%d:%d: %s (%02x:%02x)\n", PCI_TRIPLET(addr), pci_classes[class], class, subclass);
    }
}

static void pci_scan_bus(uint8_t bus) {
    for (uint8_t slot = 0; slot < 32; ++slot) {
        // Check func 0
        uint16_t vendor = pci_config_getw(PCI_ADDRESS(bus, slot, 0), PCI_CONF_VENDOR);

        if (vendor == 0xFFFF) {
            continue;
        }

        uint16_t header_type = pci_config_getw(PCI_ADDRESS(bus, slot, 0), PCI_CONF_HEADER) & 0xFF;

        pci_scan_func(PCI_ADDRESS(bus, slot, 0));

        if (header_type & 0x80) {
            for (uint8_t func = 1; func < 8; ++func) {
                if (pci_config_getw(PCI_ADDRESS(bus, slot, func), PCI_CONF_VENDOR) == 0xFFFF) {
                    break;
                }

                pci_scan_func(PCI_ADDRESS(bus, slot, func));
            }
        }
    }
}

static void pci_scan_devices(void) {
    uint8_t func;

    uint16_t header_type = pci_config_getw(PCI_ADDRESS(0, 0, 0), PCI_CONF_HEADER) & 0xFF;
    if (header_type & 0x80) {
        for (func = 0; func < 8; ++func) {
            if (pci_config_getw(PCI_ADDRESS(0, 0, func), PCI_CONF_VENDOR) == 0xFFFF) {
                break;
            }
            pci_scan_bus(func);
        }
    } else {
        pci_scan_bus(0);
    }
}

void x86_pci_init(void) {
    pci_scan_devices();
}
