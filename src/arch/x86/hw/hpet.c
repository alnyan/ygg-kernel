#include "hpet.h"
#include "sys/assert.h"
#include "sys/debug.h"
#include "sys/time.h"

#define HPET_ENABLE_CNF     (1 << 0)
#define HPET_LEG_RT_CNF     (1 << 1)

#define HPET_Tn_INT_TYPE_CNF    (1 << 1)
#define HPET_Tn_INT_ENB_CNF     (1 << 2)
#define HPET_Tn_TYPE_CNF        (1 << 3)
#define HPET_Tn_PER_INT_CAP     (1 << 4)
#define HPET_Tn_SIZE_CAP        (1 << 5)
#define HPET_Tn_VAL_SET_CNF     (1 << 6)
#define HPET_Tn_32MODE_CNF      (1 << 8)
#define HPET_Tn_INT_ROUTE_OFF   (9)
#define HPET_Tn_FSB_EN_CNF      (1 << 14)
#define HPET_Tn_FSB_INT_DEL_CAP (1 << 15)

struct hpet_timer_block {
    uint64_t ctrl;
    uint64_t value;
    uint64_t fsb_int_rt;
    uint64_t res;
};

struct hpet {
    uint64_t caps;
    uint64_t res0;
    uint64_t ctrl;
    uint64_t res1;
    uint64_t intr;
    char res2[200];
    uint64_t count;
    uint64_t res3;
    struct hpet_timer_block timers[2];
};

static struct hpet * volatile hpet = NULL;
static uint64_t hpet_systick_res;
static uint64_t hpet_freq;
static uint64_t hpet_last;

int hpet_available(void) {
    return !!hpet;
}

void hpet_set_base(uint32_t addr) {
    hpet = (struct hpet *) addr;
}

void hpet_timer_func(void) {
    // Update system-global time
    uint64_t hpet_count = hpet->count;
    systime += (hpet_count - hpet_last) / hpet_systick_res;
    hpet_last = hpet_count;

    hpet->intr |= 1;
}

int hpet_init(void) {
    debug("Initializing HPET\n");
    uint32_t hpet_clk = (hpet->caps >> 32) & 0xFFFFFFFF; // In 10^-15s
    hpet_freq = 1000000000000000 / hpet_clk;
    // TODO: specify desired frequency as function param
    uint32_t desired_freq = 1000;

    assert(hpet->timers[0].ctrl & HPET_Tn_PER_INT_CAP);

    hpet->ctrl |= HPET_ENABLE_CNF | HPET_LEG_RT_CNF;
    hpet->timers[0].ctrl = HPET_Tn_INT_ENB_CNF | HPET_Tn_VAL_SET_CNF | HPET_Tn_TYPE_CNF;
    hpet->timers[0].value = hpet_freq / desired_freq;
    hpet->timers[0].value = hpet->count + hpet_freq / desired_freq;
    hpet_last = hpet->count;
    hpet_systick_res = hpet_freq / SYSTICK_DES_RES;

    return 0;
}
