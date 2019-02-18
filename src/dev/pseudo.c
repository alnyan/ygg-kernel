#include "pseudo.h"
#include "sys/mem.h"
#include "sys/vfs.h"

dev_t *dev_null;
dev_t *dev_zero;

struct dev_pseudo {
    dev_t dev;
    int type;
};

static struct dev_pseudo dev_pseudo_null, dev_pseudo_zero;

static ssize_t dev_pseudo_read(dev_t *dev, vfs_file_t *f, void *buf, size_t len, uint32_t flags) {
    switch ((uint32_t) (f->dev_priv)) {
    case DEV_PSEUDO_NULL:
        return 0;
    case DEV_PSEUDO_ZERO:
        memset(buf, 0, len);
        return len;
    }
    return -1;
}

static ssize_t dev_pseudo_write(dev_t *dev, vfs_file_t *f, const void *buf, size_t len, uint32_t flags) {
    // Nothing
    return len;
}

void dev_pseudo_setup(void) {
    dev_init(&dev_pseudo_null.dev, DEV_FLG_RD | DEV_FLG_WR);
    dev_init(&dev_pseudo_zero.dev, DEV_FLG_RD | DEV_FLG_WR);

    dev_pseudo_zero.type = DEV_PSEUDO_ZERO;
    dev_pseudo_null.type = DEV_PSEUDO_NULL;

    dev_null = (dev_t *) &dev_pseudo_null;
    dev_zero = (dev_t *) &dev_pseudo_zero;
}
