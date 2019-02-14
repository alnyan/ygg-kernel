#include "initrd.h"

void initrd_init(dev_initrd_t *dev, uintptr_t addr, size_t len) {
    dev->base = addr;
    dev->len = len;
}
