/**
 *
 * PCI IDE Controller driver for disk interfacing
 *
 */
#include "ide.h"
#include "sys/debug.h"
#include "sys/panic.h"
#include "sys/assert.h"
#include "sys/string.h"
#include "sys/ctype.h"
#include "sys/mem.h"
#include "sys/mm.h"

// ATA command and status interface

// ATA status
#define ATA_ST_ERROR        (1 << 0)
#define ATA_ST_INDEX        (1 << 1)
#define ATA_ST_CORR         (1 << 2)
#define ATA_ST_REQC         (1 << 3)
#define ATA_ST_SEEKC        (1 << 4)
#define ATA_ST_WFAULT       (1 << 5)
#define ATA_ST_READY        (1 << 6)
#define ATA_ST_BUSY         (1 << 7)


// ATA errors
#define ATA_ERR_NOADRM      (1 << 0)
#define ATA_ERR_TRK0NE      (1 << 1)
#define ATA_ERR_ABORT       (1 << 2)
#define ATA_ERR_MCR         (1 << 3)
#define ATA_ERR_NOIDM       (1 << 4)
#define ATA_ERR_MC          (1 << 5)
#define ATA_ERR_UNCORR      (1 << 6)
#define ATA_ERR_BADBLK      (1 << 7)


// ATA commands
#define ATA_CMD_READ_PIO        0x20
#define ATA_CMD_READ_PIO_EXT    0x24
#define ATA_CMD_READ_DMA        0xC8
#define ATA_CMD_READ_DMA_EXT    0x25
#define ATA_CMD_WRITE_PIO       0x30
#define ATA_CMD_WRITE_PIO_EXT   0x34
#define ATA_CMD_WRITE_DMA       0xCA
#define ATA_CMD_WRITE_DMA_EXT   0x35
#define ATA_CMD_CFLUSH          0xE7
#define ATA_CMD_CFLUSH_EXT      0xEA
#define ATA_CMD_PACKET          0xA0
#define ATA_CMD_ID_PACKET       0xA1
#define ATA_CMD_ID              0xEC
// ATAPI commands
#define ATAPI_CMD_READ          0xA8
#define ATAPI_CMD_EJECT         0x1B


// ATA registers
#define ATA_REG_DATA            0x00
#define ATA_REG_ERROR           0x01
#define ATA_REG_FEATURE         0x01
#define ATA_REG_SECCOUNT0       0x02
#define ATA_REG_LBA0            0x03
#define ATA_REG_LBA1            0x04
#define ATA_REG_LBA2            0x05
#define ATA_REG_HDDEVSEL        0x06
#define ATA_REG_COMMAND         0x07
#define ATA_REG_STATUS          0x07
#define ATA_REG_SECCOUNT1       0x08
#define ATA_REG_LBA3            0x09
#define ATA_REG_LBA4            0x0A
#define ATA_REG_LBA5            0x0B
#define ATA_REG_CONTROL         0x0C
#define ATA_REG_ALTSTATUS       0x0C
#define ATA_REG_DEVADDR         0x0D

// IDE DMA control
#define IDE_REG_DMA_CMD         0x00
#define IDE_REG_DMA_STATUS      0x02
#define IDE_REG_DMA_PRDT        0x04

// IDE DMA control flags
#define IDE_DMA_CTL_EN          (1 << 0)
#define IDE_DMA_CTL_WR          (1 << 3)

// IDE DMA status flags
#define IDE_DMA_ST_EN           (1 << 0)
#define IDE_DMA_ST_ERR          (1 << 1)
#define IDE_DMA_ST_INT          (1 << 2)

// Channel names
#define ATA_PRIMARY             0x00
#define ATA_SECONDARY           0x01

#define ATA_IDENT_DEVICETYPE    0
#define ATA_IDENT_CYLINDERS     2
#define ATA_IDENT_HEADS         6
#define ATA_IDENT_SECTORS       12
#define ATA_IDENT_SERIAL        20
#define ATA_IDENT_MODEL         54
#define ATA_IDENT_CAPABILITIES  98
#define ATA_IDENT_FIELDVALID    106
#define ATA_IDENT_MAX_LBA       120
#define ATA_IDENT_COMMANDSETS   164
#define ATA_IDENT_MAX_LBA_EXT   200

static const uint32_t pci_ide_def_bars[] = {
    0x1F0, 0x3F6, 0x170, 0x376
};

struct ide_channel {
    uint32_t iobase;
    uint32_t ctlbase;
    uint32_t bmide;
    uint8_t noirq;
};

struct ide_dev {
    uint8_t present;
    uint8_t chan;
    uint8_t drive;
    uint8_t type;
    uint16_t sig;
    uint16_t caps;
    uint32_t command_sets;
    uint32_t size;
    char model[41];
};

struct pci_ide {
    pci_addr_t addr;
    uint32_t irq;

    // PCI DMA
    uint64_t *prdt_ptr;
    uintptr_t prdt_phys;

    uint8_t dma_pending;

    struct ide_channel chan[2];
    struct ide_dev     devs[4];
};

// TODO: support multiple IDE controllers
static struct pci_ide pci_ide;
static uint64_t pci_prdt[16] __attribute__((aligned(16)));

void ide_write(struct pci_ide *ide, uint8_t chan, uint8_t reg, uint8_t data) {
    if (reg > 0x07 && reg < 0x0C) {
        ide_write(ide, chan, ATA_REG_CONTROL, 0x80 | ide->chan[chan].noirq);
    }

    if (reg < 0x08) {
        outb(ide->chan[chan].iobase  + reg - 0x00, data);
    } else if (reg < 0x0C) {
        outb(ide->chan[chan].iobase  + reg - 0x06, data);
    } else if (reg < 0x0E) {
        outb(ide->chan[chan].ctlbase  + reg - 0x0A, data);
    } else if (reg < 0x16) {
        outb(ide->chan[chan].bmide + reg - 0x0E, data);
    }

    if (reg > 0x07 && reg < 0x0C) {
        ide_write(ide, chan, ATA_REG_CONTROL, ide->chan[chan].noirq);
    }
}

uint8_t ide_read(struct pci_ide *ide, uint8_t chan, uint8_t reg) {
    uint8_t res;

    if (reg > 0x207 && reg < 0x0C) {
        ide_write(ide, chan, ATA_REG_CONTROL, 0x80 | ide->chan[chan].noirq);
    }

    if (reg < 0x08) {
        res = inb(ide->chan[chan].iobase  + reg - 0x00);
    } else if (reg < 0x0C) {
        res = inb(ide->chan[chan].iobase  + reg - 0x06);
    } else if (reg < 0x0E) {
        res = inb(ide->chan[chan].ctlbase  + reg - 0x0A);
    } else if (reg < 0x16) {
        res = inb(ide->chan[chan].bmide + reg - 0x0E);
    }

    if (reg > 0x07 && reg < 0x0C) {
        ide_write(ide, chan, ATA_REG_CONTROL, ide->chan[chan].noirq);
    }

    return res;
}

void ide_read_block(struct pci_ide *ide, uint8_t chan, uint8_t reg, uint32_t *buf, size_t count) {
    if (reg > 0x207 && reg < 0x0C) {
        ide_write(ide, chan, ATA_REG_CONTROL, 0x80 | ide->chan[chan].noirq);
    }

    for (size_t i = 0; i < count; ++i) {
        if (reg < 0x08) {
            buf[i] = inl(ide->chan[chan].iobase  + reg - 0x00);
        } else if (reg < 0x0C) {
            buf[i] = inl(ide->chan[chan].iobase  + reg - 0x06);
        } else if (reg < 0x0E) {
            buf[i] = inl(ide->chan[chan].ctlbase  + reg - 0x0A);
        } else if (reg < 0x16) {
            buf[i] = inl(ide->chan[chan].bmide + reg - 0x0E);
        }
    }

    if (reg > 0x07 && reg < 0x0C) {
        ide_write(ide, chan, ATA_REG_CONTROL, ide->chan[chan].noirq);
    }
}

int pci_ide_dma_read(struct pci_ide *ide, uint8_t chan, uint8_t drive, uint32_t lba, uintptr_t buf_phys, uint8_t nsec) {
    assert(nsec < 127);
    assert(!(ide->dma_pending));

    kdebug("ide dma read lba %p, buf %p, siz %u\n", lba, buf_phys, nsec);

    uint16_t flag = 0x8000;
    uint16_t size = nsec * 512;
    uint32_t addr = buf_phys;

    ide->dma_pending = 1;

    pci_prdt[0] = ((uint64_t) flag << 48) | (((uint64_t) size) << 32) | (addr);

    // Start DMA
    // Write PRDT address to the descriptor
    outl(ide->chan[chan].bmide + IDE_REG_DMA_PRDT, ide->prdt_phys);
    // Read direction
    outb(ide->chan[chan].bmide + IDE_REG_DMA_CMD, 0);
    inb(ide->chan[chan].bmide + IDE_REG_DMA_STATUS);
    outb(ide->chan[chan].bmide + IDE_REG_DMA_STATUS, 0);
    // Write LBA
    uint8_t lba_io[6];
    uint8_t head;

    lba_io[0] = lba & 0xFF;
    lba_io[1] = (lba >> 8) & 0xFF;
    lba_io[2] = (lba >> 16) & 0xFF;
    lba_io[3] = 0;
    lba_io[4] = 0;
    lba_io[5] = 0;
    head = (lba >> 24) & 0xF;

    // Wait until the drive is free to serve us
    while (ide_read(ide, chan, ATA_REG_STATUS) & ATA_ST_BUSY) {
    }

    // Select drive
    ide_write(ide, chan, ATA_REG_HDDEVSEL, 0xE0 | (drive << 4) | head);

    // Write LBA and sector count
    ide_write(ide, chan, ATA_REG_SECCOUNT0, nsec);
    ide_write(ide, chan, ATA_REG_LBA0, lba_io[0]);
    ide_write(ide, chan, ATA_REG_LBA1, lba_io[1]);
    ide_write(ide, chan, ATA_REG_LBA2, lba_io[2]);

    // Issue DMA read command
    ide_write(ide, chan, ATA_REG_COMMAND, ATA_CMD_READ_DMA);

    // Set DMA status bit in ctl reg
    outb(ide->chan[chan].bmide + IDE_REG_DMA_CMD, IDE_DMA_CTL_EN);

    return 0;
}

int pci_ide_irq_handler(int irq) {
    uint8_t handled = 0;
    uint8_t status[2];

    for (int chan = 0; chan < 2; ++chan) {
        status[chan] = inb(pci_ide.chan[chan].bmide + IDE_REG_DMA_STATUS);

        // This means some other device issued an IRQ
        if (!(status[chan] & IDE_DMA_ST_INT)) {
            continue;
        }

        handled |= (1 << chan);
    }

    if (handled) {
        if (handled & (1 << 0)) {
            kdebug("Handled master IDE bus IRQ\n");

            if (status[0] & IDE_DMA_ST_ERR) {
                kdebug("Master IDE bus had an error\n");
            }

            if (!(status[0] & IDE_DMA_ST_EN) && pci_ide.dma_pending) {
                kdebug("Master IDE DMA operation finished\n");
            }
        }

        return 0;
    }

    return -1;
}

int pci_ide_init(pci_addr_t addr) {
    // Enable PCI busmastering for the controller
    uint32_t cmd_reg = pci_config_getw(addr, PCI_CONF_COMMAND);
    cmd_reg |= (1 << 2);
    pci_config_setw(addr, PCI_CONF_COMMAND, cmd_reg);
    cmd_reg = pci_config_getw(addr, PCI_CONF_COMMAND);

    // PCI DMA
    pci_ide.prdt_ptr = pci_prdt;
    pci_ide.prdt_phys = mm_translate(mm_kernel, (uintptr_t) pci_ide.prdt_ptr, NULL);
    assert(pci_ide.prdt_phys != MM_NADDR);
    pci_ide.dma_pending = 0;

    pci_ide.addr = addr;
    pci_ide.irq = 14;

    uint32_t bar4 = pci_config_getl(addr, PCI_CONF_BAR0 + 4 * 4) & ~1;

    for (int i = 0; i < 2; ++i) {
        pci_ide.chan[i].iobase = pci_config_getl(addr, PCI_CONF_BAR0 + i * 8);
        if (pci_ide.chan[i].iobase < 0x2) {
            pci_ide.chan[i].iobase = pci_ide_def_bars[i * 2];
        }
        pci_ide.chan[i].ctlbase = pci_config_getl(addr, PCI_CONF_BAR0 + i * 8 + 4);
        if (pci_ide.chan[i].ctlbase < 0x2) {
            pci_ide.chan[i].ctlbase = pci_ide_def_bars[i * 2 + 1];
        }
    }

    pci_ide.chan[ATA_PRIMARY].bmide = bar4;
    pci_ide.chan[ATA_SECONDARY].bmide = bar4 + 8;

    pci_ide.chan[ATA_PRIMARY].noirq = 0;
    pci_ide.chan[ATA_SECONDARY].noirq = 0;

    // Disable IRQs
    ide_write(&pci_ide, ATA_PRIMARY, ATA_REG_CONTROL, 2);
    ide_write(&pci_ide, ATA_SECONDARY, ATA_REG_CONTROL, 2);

    // Obtain drive info
    memset(pci_ide.devs, 0, sizeof(struct ide_dev) * 4);
    char id_buf[512];
    for (int ch = 0; ch < 2; ++ch) {
        for (int n = 0; n < 2; ++n) {
            struct ide_dev *dev = &pci_ide.devs[ch * 2 + n];
            uint8_t r, e = 0;

            // Select drive n on channel ch
            ide_write(&pci_ide, ch, ATA_REG_HDDEVSEL, 0xA0 | (n << 4));
            for (int i = 0; i < 10; ++i) {
                ide_read(&pci_ide, ch, ATA_REG_ALTSTATUS);
            }

            // Request identification
            ide_write(&pci_ide, ch, ATA_REG_COMMAND, ATA_CMD_ID);
            for (int i = 0; i < 10; ++i) {
                ide_read(&pci_ide, ch, ATA_REG_ALTSTATUS);
            }

            if (ide_read(&pci_ide, ch, ATA_REG_STATUS) == 0) {
                // No drive found, status == 0
                continue;
            }

            while (1) {
                r = ide_read(&pci_ide, ch, ATA_REG_STATUS);
                if (r & ATA_ST_ERROR) {
                    e = 1;
                    break;
                }
                if ((!(r & ATA_ST_BUSY)) && (r & ATA_ST_READY)) {
                    break;
                }
            }

            // TODO: probe ATAPI drives
            if (e) {
                continue;
            }

            ide_read_block(&pci_ide, ch, ATA_REG_DATA, (uint32_t *) id_buf, sizeof(id_buf) / 4);

            dev->present = 1;
            dev->chan = ch;
            dev->drive = n;
            dev->type = 0;
            dev->sig = *((uint16_t *) (&id_buf[ATA_IDENT_DEVICETYPE]));
            dev->caps = *((uint16_t *) (&id_buf[ATA_IDENT_CAPABILITIES]));
            dev->command_sets = *((uint32_t *) (&id_buf[ATA_IDENT_COMMANDSETS]));
            strncpy(dev->model, &id_buf[ATA_IDENT_MODEL], 40);
            // Swap byte order
            for (int i = 0; i < 40; i += 4) {
                *((uint32_t *) (&dev->model[i])) = ((0xFF & dev->model[i + 0]) << 8) |
                                                   ((0xFF & dev->model[i + 1]) << 0) |
                                                   ((0xFF & dev->model[i + 2]) << 24) |
                                                   ((0xFF & dev->model[i + 3]) << 16);
            }
            dev->model[40] = 0;
            // May contain trailing spaces, so need to cut them off
            for (int i = 39; i >= 0; --i) {
                if (isspace(dev->model[i])) {
                    dev->model[i] = 0;
                } else {
                    break;
                }
            }

            kdebug("Found IDE drive: %s\n", dev->model);
        }
    }

    return 0;
}
