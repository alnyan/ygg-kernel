#include "cpuid.h"
#include "sys/assert.h"
#include "sys/debug.h"
#include "sys/mem.h"
#include "sys/assert.h"

#define CPUID_EAX_VENDOR    0x00
#define CPUID_EAX_FEATURES  0x01
#define CPUID_EAX_TLBCACHE  0x02
#define CPUID_EAX_SERIAL    0x03
#define CPUID_EAX_BRAND0    0x8000002

extern int x86_cpuid_support(void);

static const char *x86_cpuid_ecx_feature_names[] = {
    "sse3",
    "pclmul",
    "dtes64",
    "monitor",
    "ds_cpl",
    "vmx",
    "smx",
    "est",
    "tm2",
    "ssse3",
    "cid",
    NULL,
    "fma",
    "cx16",
    "etprd",
    "pdcm",
    "pcide",
    "dca",
    "sse4.1",
    "sse4.2",
    "x2-apic",
    "movbe",
    "popcnt",
    "aes",
    "xsave",
    "osxsave",
    "avx"
};

static const char *x86_cpuid_edx_feature_names[] = {
    "fpu",
    "vme",
    "de",
    // TODO: PSE should be checked at the very start of the kernel
    "pse",
    "tsc",
    "msr",
    "pae",
    "mce",
    "cx8",
    "apic",
    "sep",
    "mtrr",
    "pge",
    "mca",
    "cmov",
    "pat",
    "pse36",
    "psn",
    "clf",
    NULL,
    "dtes",
    "acpi",
    "mmx",
    "fxsr",
    "sse",
    "sse2",
    "ss",
    "htt",
    "tm1",
    "ia64",
    "pbe"
};


static char s_cpuid_vendor[13];
static char s_cpuid_brand[49];
static uint32_t s_cpuid_feature_edx = 0;
static uint32_t s_cpuid_feature_ecx = 0;

static void x86_cpuid(uint32_t eax, uint32_t volatile *res) {
    asm volatile ("cpuid":
            "=a"(*(res + 0)),
            "=b"(*(res + 1)),
            "=c"(*(res + 2)),
            "=d"(*(res + 3)):
            "a"(eax));
}

int x86_cpuid_check_feature(uint32_t num) {
    if (num <= 32) {
        return (s_cpuid_feature_ecx & (1 << num));
    } else {
        return (s_cpuid_feature_edx & (1 << (uint32_t) (num - 32)));
    }
}

void x86_cpuid_dump_features(void) {
    kdebug("CPUID: Vendor \"%s\"\n", s_cpuid_vendor);
    kdebug("Brand string: \"%s\"\n", s_cpuid_brand);
    kdebug("EDX features:\n");
    for (uint32_t i = 0; i < sizeof(x86_cpuid_edx_feature_names) / sizeof(x86_cpuid_edx_feature_names[0]); ++i) {
        if (((1 << i) & s_cpuid_feature_edx) && x86_cpuid_edx_feature_names[i]) {
            kdebug("%s\n", x86_cpuid_edx_feature_names[i]);
        }
    }
    kdebug("ECX features:\n");
    for (uint32_t i = 0; i < sizeof(x86_cpuid_ecx_feature_names) / sizeof(x86_cpuid_ecx_feature_names[0]); ++i) {
        if (((1 << i) & s_cpuid_feature_ecx) && x86_cpuid_ecx_feature_names[i]) {
            kdebug("%s\n", x86_cpuid_ecx_feature_names[i]);
        }
    }
}

void x86_cpuid_init(void) {
    assert(x86_cpuid_support());

    kdebug("CPUID support is available\n");

    uint32_t res[4];

    memset(res, 0, 4 * 4);
    // Query features
    x86_cpuid(CPUID_EAX_FEATURES, res);
    s_cpuid_feature_edx = res[3];
    s_cpuid_feature_ecx = res[2];

    // Request CPU vendor
    x86_cpuid(CPUID_EAX_VENDOR, res);
    s_cpuid_vendor[12] = 0;
    ((uint32_t *) s_cpuid_vendor)[0] = res[1];
    ((uint32_t *) s_cpuid_vendor)[1] = res[3];
    ((uint32_t *) s_cpuid_vendor)[2] = res[2];

    // Query brand string
    s_cpuid_brand[48] = 0;
    for (int i = 0; i < 3; ++i) {
        x86_cpuid(CPUID_EAX_BRAND0 + i, res);
        ((uint32_t *) s_cpuid_brand)[i * 4 + 0] = res[0];
        ((uint32_t *) s_cpuid_brand)[i * 4 + 1] = res[1];
        ((uint32_t *) s_cpuid_brand)[i * 4 + 2] = res[2];
        ((uint32_t *) s_cpuid_brand)[i * 4 + 3] = res[3];
    }

    x86_cpuid_dump_features();

    // Assert basic features
    assert(
        x86_cpuid_check_feature(CPUID_FEAT_FPU)
    );
}
