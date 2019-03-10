#include "initrd.h"
#include "sys/mem.h"

struct initrd {
    dev_t dev;
    uintptr_t base;
};

static struct initrd initrd;
dev_t *dev_initrd = &initrd.dev;

static int initrd_would_block(dev_t *dev, uintptr_t pos, size_t count, int dir) {
    // The initrd is located directly in RAM, so it won't block
    return 0;
}

static ssize_t initrd_read_imm(dev_t *dev, void *dst, uintptr_t pos, size_t req) {
    memcpy(dst, (const void *) (((struct initrd *) dev)->base + pos), req);
    return req;
}

static uintptr_t initrd_getm(dev_t *dev, uintptr_t pos, size_t count) {
    return ((struct initrd *) dev)->base + pos;
}

void initrd_init(uintptr_t base) {
    dev_init(dev_initrd, DEV_TYPE_BLK, DEV_FLG_MEMR);

    initrd.dev.would_block = initrd_would_block;
    initrd.dev.read_imm = initrd_read_imm;
    initrd.dev.getm = initrd_getm;

    initrd.base = base;
}
