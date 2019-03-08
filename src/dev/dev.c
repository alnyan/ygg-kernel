#include "dev.h"
#include "sys/assert.h"
#include "sys/mem.h"

void dev_init(dev_t *dev, int type) {
    assert(!(type & ~0x1));
    assert(dev);

    memset(dev, 0, sizeof(dev_t));
    dev->flags = type;
}
