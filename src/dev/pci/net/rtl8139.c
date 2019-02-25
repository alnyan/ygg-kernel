#include "rtl8139.h"
#include "sys/debug.h"
#include "sys/panic.h"
#include "sys/assert.h"
#include "sys/mm.h"
#include "sys/mem.h"
#include "net/eth/eth.h"
#include "net/dhcp.h"
#include "net/udp.h"

#define IGNORE(x)

// TODO: automatic device memory allocation
// Just allocate a virtual page to map devices
#define RTL8139_PAGE        0xFDC00000

#define RTL8139_ISR_ROK     (1 << 0)
#define RTL8139_ISR_RER     (1 << 1)
#define RTL8139_ISR_TOK     (1 << 2)
#define RTL8139_ISR_TER     (1 << 3)

#define RTL8139_CR_TE       (1 << 2)
#define RTL8139_CR_RE       (1 << 3)

#define RTL8139_RCR_AAP     (1 << 0)
#define RTL8139_RCR_APM     (1 << 1)
#define RTL8139_RCR_AM      (1 << 2)
#define RTL8139_RCR_AB      (1 << 3)
#define RTL8139_RCR_AR      (1 << 4)
#define RTL8139_RCR_AER     (1 << 5)

#define RTL8139_STATUS_TXOP (1 << 7)

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
static char rtl8139_txbuf[4096];

// Does not inherit dev_t
struct rtl8139 {
    uint32_t iobase;
    uint32_t membase;
    uint16_t cbr_prev;
    uint8_t int_no;
    uint8_t tx_reg_num;

    // Lower 4 bits:
    //  1 - txbuf allocated, 0 - txbuf free
    // Upper 4 bits:
    //  1 - txbuf has txop scheduled, 0 - no pending ops on buffer
    uint8_t status;

    size_t buflen[4];
    struct rtl8139_registers *regs;
} rtl8139;

static void rtl8139_recv(int s) {
    uint16_t size;

    if (rtl8139.regs->cbr >= rtl8139.cbr_prev) {
        size = rtl8139.regs->cbr - rtl8139.cbr_prev;
    } else {
        size = rtl8139.regs->cbr;
    }

    if (size >= (4 + sizeof(struct eth_hdr))) {
        eth_handle_packet(rtl8139.cbr_prev + rtl8139_recv_buf + 4, size - 4);
    }

    rtl8139.cbr_prev = rtl8139.regs->cbr;
}

static int rtl8139_alloc_txbuf(void) {
    for (int d = 0, i = rtl8139.tx_reg_num; d < 4; i = (i + 1) & 0x3, ++d) {
        if (rtl8139.status & (1 << i)) {
            continue;
        }

        // Mark "slot" as used
        rtl8139.status |= (1 << i);

        return i;
    }

    // No free "slots" left
    return -1;
}

static void rtl8139_tx_sched(void) {
    // If we're not busy Tx'ing
    if (!(rtl8139.status & RTL8139_STATUS_TXOP)) {
        assert(rtl8139.status & (1 << rtl8139.tx_reg_num));
        kdebug("tx #%u\n", rtl8139.tx_reg_num);
        rtl8139.regs->tsd[rtl8139.tx_reg_num] = rtl8139.buflen[rtl8139.tx_reg_num];
        rtl8139.tx_reg_num = (rtl8139.tx_reg_num + 1) & 0x3;
        rtl8139.status |= RTL8139_STATUS_TXOP;
    }
}

int rtl8139_send(const void *buf, size_t size) {
    if (size > 1024) {
        return -1;
    }

    int txbuf = rtl8139_alloc_txbuf();

    if (txbuf == -1) {
        kdebug("Tx queue overflow\n");
        return -1;
    }

    rtl8139.status |= (1 << txbuf);
    rtl8139.buflen[txbuf] = size & 0x0FFF;

    kdebug("sched tx #%u\n", txbuf);
    memcpy(&rtl8139_txbuf[1024 * txbuf], buf, size);

    if (rtl8139.tx_reg_num == txbuf) {
        // Perform the send right now
        rtl8139_tx_sched();
    }

    return 0;
}

int rtl8139_irq_handler(int irq) {
    uint32_t status = rtl8139.regs->isr;

    if (status & RTL8139_ISR_ROK) {
        kinfo("Recv OK\n");
        rtl8139.regs->isr |= RTL8139_ISR_ROK;
        rtl8139_recv(0);
    } else if (status & RTL8139_ISR_TOK) {
        // Mark txop as complete
        int txop = ((int) rtl8139.tx_reg_num - 1) & 0x3;
        kdebug("marking #%d\n", txop);

        assert(rtl8139.status & (1 << txop));
        // Also clear busy bit
        rtl8139.status &= ~((1 << txop) | RTL8139_STATUS_TXOP);
        rtl8139.regs->isr |= RTL8139_ISR_TOK;
        kinfo("Transmit OK\n");
        // Sched if next is set
        if (rtl8139.status & (1 << rtl8139.tx_reg_num)) {
            rtl8139_tx_sched();
        }
    } else if (status & RTL8139_ISR_RER) {
        kinfo("Recv ERR\n");
        rtl8139.regs->isr |= RTL8139_ISR_RER;
    } else {
        kinfo("Unhandled status\n");
        return -1;
    }

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
    rtl8139.cbr_prev = 0;
    rtl8139.int_no = (uint8_t) (pci_config_getw(addr, PCI_CONF_INTFO) & 0xFF);

    kinfo("Interrupt line: %u\n", (uint32_t) rtl8139.int_no);

    assert(mm_map_page(mm_kernel, RTL8139_PAGE, rtl8139.membase & -MM_PAGESZ, MM_FLG_RW) == 0);
    rtl8139.regs = (struct rtl8139_registers *) (RTL8139_PAGE | (rtl8139.membase & 0x3FFFFF));

    // Turn on RTL8139
    rtl8139.regs->config1 = 0;
    io_wait();

    // Reset
    rtl8139.regs->cr = 0x10;
    while ((rtl8139.regs->cr & 0x10)) {}

    uint8_t mac[6];
    memcpy(mac, rtl8139.regs->idr, 6);
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

    // Setup txbufs
    for (int i = 0; i < 4; ++i) {
        uint32_t txbuf_phys = mm_lookup(mm_kernel, (uintptr_t) &rtl8139_txbuf[i * 1024], MM_FLG_HUGE);
        assert(txbuf_phys != MM_NADDR);
        rtl8139.regs->tsad[i] = txbuf_phys;
    }

    // Configure interrupt mask
    rtl8139.regs->imr = RTL8139_ISR_ROK | RTL8139_ISR_RER | RTL8139_ISR_TOK | RTL8139_ISR_TER;

    // Configure recv settings
    rtl8139.regs->rcr = (1 << 0) |
                        (1 << 1) |
                        (1 << 2) |
                        (1 << 3) |
                        (1 << 4) |
                        (1 << 5);

    // Enable Rx/Tx
    rtl8139.regs->cr = RTL8139_CR_TE | RTL8139_CR_RE;

    rtl8139.status = 0;

    return -1;
}
