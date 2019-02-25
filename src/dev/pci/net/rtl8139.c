#include "rtl8139.h"
#include "sys/debug.h"
#include "sys/panic.h"
#include "sys/assert.h"
#include "sys/mm.h"

#define MAC_FMT             "%02x:%02x:%02x:%02x:%02x:%02x"

// TODO: automatic device memory allocation
// Just allocate a virtual page to map devices
#define RTL8139_PAGE        0xFDC00000

struct rtl8139_registers {
    uint8_t idr[6];
    uint8_t res0[2];
    uint8_t mar[8];
    uint32_t tsd[4];
    uint32_t tsad[4];
    uint32_t rbstart;
    uint16_t erbcr;
    uint8_t ersr;
    uint8_t cr;
    uint16_t capr;
    uint16_t cbr;
    uint16_t imr;
    uint16_t isr;
    uint32_t tcr;
    uint32_t rcr;
    uint32_t tctr;
    uint32_t mpc;
    uint8_t _9346cr;
    uint8_t config0;
    uint8_t config1;
    uint8_t res1;
    uint32_t timer_int;
    uint8_t msr;
    uint8_t config3;
    uint8_t config4;
    uint8_t res2;
    uint16_t mulint;
    // ...
};

static char rtl8139_recv_buf[8192 + 16];

// Does not inherit dev_t
struct rtl8139 {
    uint32_t iobase;
    uint32_t membase;
    uint8_t int_no;
    struct rtl8139_registers *regs;
} rtl8139;

int rtl8139_irq_handler(int irq) {
    kinfo("RTL8139 got IRQ\n");

    rtl8139.regs->isr = 1;

    return 0;
}

int rtl8139_init(pci_addr_t addr) {
    kinfo("Initializing Realtek RTL8139 at " PCI_FMTADDR "\n", PCI_TRIPLET(addr));

    // Enable PCI busmastering for NIC
    uint32_t cmd_reg = pci_config_getw(addr, PCI_CONF_COMMAND);
    kinfo("cmd_reg = %p\n", cmd_reg);
    cmd_reg |= (1 << 2);
    pci_config_setw(addr, PCI_CONF_COMMAND, cmd_reg);
    cmd_reg = pci_config_getw(addr, PCI_CONF_COMMAND);
    kinfo("cmd_reg = %p\n", cmd_reg);

    rtl8139.iobase = pci_config_getl(addr, PCI_CONF_BAR0);
    if (rtl8139.iobase & 1) {
        rtl8139.iobase &= 0xFFFFFFF0;
    } else {
        panic("Unhandled case: IOIN == 0\n");
    }

    rtl8139.membase = pci_config_getl(addr, PCI_CONF_BAR1);
    if (rtl8139.membase & 1) {
        panic("Unhandled case: MEMIN == 1\n");
    }
    rtl8139.membase &= 0xFFFFFFF0;

    rtl8139.int_no = (uint8_t) (pci_config_getw(addr, PCI_CONF_INTFO) & 0xFF);

    kinfo("Interrupt line: %u\n", (uint32_t) rtl8139.int_no);
    kinfo("IOAR: %p\n", rtl8139.iobase);
    kinfo("MEMAR: %p\n", rtl8139.membase);

    assert(mm_map_page(mm_kernel, RTL8139_PAGE, rtl8139.membase & -MM_PAGESZ, MM_FLG_RW) == 0);
    rtl8139.regs = (struct rtl8139_registers *) (RTL8139_PAGE | (rtl8139.membase & 0x3FFFFF));

    // Turn on RTL8139
    rtl8139.regs->config1 = 0;
    io_wait();

    // Reset
    rtl8139.regs->cr = 0x10;
    while ((rtl8139.regs->cr & 0x10)) {}

    kinfo("IDR: " MAC_FMT "\n", rtl8139.regs->idr[0],
                                rtl8139.regs->idr[1],
                                rtl8139.regs->idr[2],
                                rtl8139.regs->idr[3],
                                rtl8139.regs->idr[4],
                                rtl8139.regs->idr[5]);

    uintptr_t recvbuf_phys = mm_lookup(mm_kernel, (uintptr_t) rtl8139_recv_buf, MM_FLG_HUGE);
    assert(recvbuf_phys != MM_NADDR);

    // Set recvbuf address
    rtl8139.regs->rbstart = recvbuf_phys;

    assert(offsetof(struct rtl8139_registers, rbstart) == 0x30);
    assert(offsetof(struct rtl8139_registers, rcr) == 0x44);
    assert(offsetof(struct rtl8139_registers, cr) == 0x37);
    assert(offsetof(struct rtl8139_registers, config1) == 0x52);

    // Configure interrupt mask
    rtl8139.regs->imr = (1 << 0) | (1 << 1);

    // Configure recv settings
    rtl8139.regs->rcr = (1 << 0) |
                        (1 << 1) |
                        (1 << 2) |
                        (1 << 3) |
                        (1 << 4) |
                        (1 << 5) |
                        (1 << 7);
    io_wait();

    // Enable Rx
    rtl8139.regs->cr = (1 << 3);
    kinfo("STATUS = %p\n", rtl8139.regs->config3);
    io_wait();

    return -1;
}
