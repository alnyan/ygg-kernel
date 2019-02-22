#pragma once

static const char *pci_classes[] = {
    "Unknown",
    "Mass storage",
    "Networking",
    "Display",
    "Multimedia",
    "Memory",
    "Bridge",
    "Simple communication",
    "Base system",
    "Input",
    "Docking station",
    "Processor",
    "Serial bus",
    "Wireless",
    "Intelligent",
    "Satellite communication",
    "Encryption",
    "Signal processing",
};

#define PCI_CLASS_STORAGE       0x01
#define PCI_CLASS_NETWORK       0x02
#define PCI_CLASS_DISPLAY       0x03
#define PCI_CLASS_MULTIMEDIA    0x04
#define PCI_CLASS_MEMORY        0x05
#define PCI_CLASS_BRIDGE        0x06

// Most common vendor list
#define PCI_VENDOR_INTEL        0x8086

