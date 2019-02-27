#include "acpi.h"
#include <stdint.h>
#include "sys/debug.h"
#include "arch/hw.h"
#include "sys/string.h"
#include "sys/mem.h"
#include "sys/assert.h"
#include "sys/panic.h"
#include "sys/mm.h"
#include "rtc.h"
#include "hpet.h"

#define ACPI_MAP_VIRT       0xFF000000
#define HPET_MAP_VIRT       0xFE000000

struct acpi_rsdp {
    char signature[8];
    uint8_t checksum;
    char oemid[6];
    uint8_t rev;
    uint32_t rsdt;
    uint32_t length;
    char unused[12];
};

struct acpi_table_header {
    char signature[4];
    uint32_t length;
    uint8_t rev;
    uint8_t checksum;
    char oemid[6];
    char oemtableid[8];
    uint32_t oemrev;
    uint32_t creatid;
    uint32_t creatrev;
};

struct acpi_gas {
    uint8_t space;
    uint8_t bit_width;
    uint8_t bit_offset;
    uint8_t res;
    uint64_t addr;
};

struct acpi_rsdt {
    struct acpi_table_header hdr;
};

struct acpi_fadt {
    struct acpi_table_header hdr;
    uint32_t firmware_ctrl;
    uint32_t dsdt;
    uint8_t res0;
    uint8_t pref_pm_profile;
    uint16_t sci_int;
    uint32_t smi_cmd;
    uint8_t acpi_enable;
    uint8_t acpi_disable;
    uint8_t s4bios_req;
    uint8_t pstate_cnt;
    uint32_t pm1a_evt_blk;
    uint32_t pm1b_evt_blk;
    uint32_t pm1a_cnt_blk;
    uint32_t pm1b_cnt_blk;
    uint32_t pm2_cnt_blk;
    uint32_t pm_tmr_blk;
    uint32_t gpe0_blk;
    uint32_t gpe1_blk;
    uint8_t pm1_evt_len;
    uint8_t pm1_cnt_len;
    uint8_t pm2_cnt_len;
    uint8_t pm_tmr_len;
    uint8_t gpe0_blk_len;
    uint8_t gpe1_blk_len;
    uint8_t gpe1_base;
    uint8_t cst_cnt;
    uint16_t p_lvl2_lat;
    uint16_t p_lvl3_lat;
    uint16_t flush_size;
    uint16_t flush_stride;
    uint8_t duty_offset;
    uint8_t duty_width;
    uint8_t day_alrm;
    uint8_t mon_alrm;
    uint8_t century;
    uint16_t iapc_boot_arch;
    uint8_t res1;
    uint32_t flags;
    struct acpi_gas reset_reg;
    uint8_t reset_value;
    char res2[3];
} __attribute__((packed));

struct acpi_facs {
    char signature[4];
    uint32_t length;
    uint32_t hw_signature;
    uint32_t fw_waking_vec;
    uint32_t global_lock;
    uint32_t flags;
};

struct acpi_hpet {
    struct acpi_table_header hdr;
    uint32_t evt_timer_blk_id;
    struct acpi_gas base_addr;
    uint8_t hpet_number;
    uint16_t min_clock_tick;
    uint8_t oemattr;
};

static int acpi_validate_rsdp(uintptr_t addr) {
    size_t len = ((struct acpi_rsdp *) addr)->length;
    if (len > 36) {
        return -1;
    }

    uint8_t sum = 0;
    for (size_t i = 0; i < len; ++i) {
        sum += ((uint8_t *) addr)[i];
    }

    return sum != 0;
}

static int acpi_validate_header(uintptr_t addr) {
    size_t len = ((struct acpi_table_header *) addr)->length;
    uint8_t sum = 0;
    for (size_t i = 0; i < len; ++i) {
        sum += ((uint8_t *) addr)[i];
    }
    return sum != 0;
}

int x86_acpi_init(void) {
    assert(sizeof(struct acpi_rsdp) == 36);

    // Try to find the ACPI tables
    uintptr_t ebda = ((uint32_t) (*(uint16_t *) (0x040E + KERNEL_VIRT_BASE)) << 4) + KERNEL_VIRT_BASE;
    uintptr_t rsdt = 0;

    for (uintptr_t addr = ebda; addr < 0xA0000 - 4; addr += 4) {
        if (!strncmp((const char *) addr, "RSD PTR ", 8)) {
            if (acpi_validate_rsdp(addr) == 0) {
                rsdt = ((struct acpi_rsdp *) addr)->rsdt;
                break;
            }
        }
    }

    for (uintptr_t addr = 0xE0000 + KERNEL_VIRT_BASE;
        addr < 0x100000 + KERNEL_VIRT_BASE - 4; addr += 4) {
        if (!strncmp((const char *) addr, "RSD PTR ", 8)) {
            if (acpi_validate_rsdp(addr) == 0) {
                rsdt = ((struct acpi_rsdp *) addr)->rsdt;
                break;
            }
        }
    }

    assert(rsdt != 0);

    uint32_t rsdt_p = rsdt;
    kdebug("RSDT physical address is %p\n", rsdt);
    // Map RSDT
    mm_map_page(mm_kernel, ACPI_MAP_VIRT, rsdt & -MM_PAGESZ, MM_FLG_RW | MM_FLG_HUGE);
    // TODO: either copy this data or make sure the data does not get overwritten
    rsdt = (rsdt & 0x3FFFFF) | ACPI_MAP_VIRT;
    kdebug("RSDT virtual address is %p\n", rsdt);

    // Validate rsdt
    if (strncmp((const char *) rsdt, "RSDT", 4)) {
        panic("rsdt pointer does not have RSDT signature\n");
    }
    assert(acpi_validate_header(rsdt) == 0);
    struct acpi_rsdt *rsdt_s = (struct acpi_rsdt *) rsdt;

    kdebug("RSDT contains %u addresses\n", (rsdt_s->hdr.length - sizeof(struct acpi_rsdt)) / 4);

    // These pointers will be extra-checked, just useful info dump for later times
    uintptr_t hpet = MM_NADDR;
    uintptr_t fadt = MM_NADDR;
    uintptr_t apic = MM_NADDR;
    uintptr_t facs = MM_NADDR;

    // For now, just look up everything
    for (int i = 0; i < (rsdt_s->hdr.length - sizeof(struct acpi_rsdt)) / 4; ++i) {
        uint32_t phys_addr = ((uint32_t *) (rsdt + 36))[i];

        if ((phys_addr & -0x400000) != (rsdt_p & -0x400000)) {
            panic("Unhandled case\n");
        }

        uint32_t virt_addr = (phys_addr & 0x3FFFFF) | ACPI_MAP_VIRT;

        if (acpi_validate_header(virt_addr) != 0) {
            kdebug(" [%d] %p: INVALID TABLE\n", i, virt_addr);
            continue;
        }

        // Just try to display first 4 bytes, maybe a signature
        char bytes[5];
        bytes[4] = 0;
        memcpy(bytes, (const char *) virt_addr, 4);

        if (!strncmp(bytes, "FACP", 4)) {
            fadt = virt_addr;
        } else if (!strncmp(bytes, "HPET", 4)) {
            hpet = virt_addr;
        } else if (!strncmp(bytes, "APIC", 4)) {
            apic = virt_addr;
        }

        kdebug(" [%d] %p: \"%s\"\n", i, virt_addr, bytes);
    }

    if (fadt != MM_NADDR) {
        struct acpi_fadt *fadt_s = (struct acpi_fadt *) fadt;

        kdebug("FADT summary:\n");

        if (fadt_s->century != 0) {
            kdebug(" * RTC supports Century register\n");
            x86_rtc_set_century_addr(fadt_s->century);
        }

        if ((facs = fadt_s->firmware_ctrl)) {
            if ((facs & -0x400000) != (rsdt_p & -0x400000)) {
                panic("Unhandled case\n");
            }

            facs = (facs & 0x3FFFFF) | ACPI_MAP_VIRT;

            if (strncmp((const char *) facs, "FACS", 4)) {
                panic("FACS table is missing a signature\n");
            }
        }
    }

    if (hpet != MM_NADDR) {
        struct acpi_hpet *hpet_s = (struct acpi_hpet *) hpet;
        kdebug("HPET summary:\n");
        kdebug(" * Base addr: %c:%lp\n", hpet_s->base_addr.space ? 'I' : 'M', hpet_s->base_addr.addr);
        assert(hpet_s->base_addr.space == 0 /* Non-memory mappings not supported yet */);

        uint32_t hpet_addr = hpet_s->base_addr.addr;

        assert(!(mm_kernel[HPET_MAP_VIRT >> 22] & 1));
        mm_map_page(mm_kernel, HPET_MAP_VIRT, hpet_addr & -MM_PAGESZ, MM_FLG_RW | MM_FLG_HUGE);

        hpet_addr = (hpet_addr & 0x3FFFFF) | HPET_MAP_VIRT;

        kdebug(" * HPET virtual mapped addr: %p\n", hpet_addr);

        hpet_set_base(hpet_addr);
    }

    return -1;
}
