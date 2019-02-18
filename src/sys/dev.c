#include "dev.h"
#include "sys/assert.h"
#include "sys/debug.h"
#include "sys/vfs.h"
#include "sys/mem.h"

void dev_init(dev_t *dev, uint32_t flags) {
    memset(dev, 0, sizeof(dev_t));

    dev->flags = flags;
}

int dev_open(dev_t *dev, vfs_file_t *f, uint32_t flags) {
    assert(dev);

    // Set appropriate flags
    f->flags &= ~(7 << 2);
    if (dev->flags & DEV_FLG_CHR) {
        f->flags |= (VFS_TYPE_CHR << 2);
    } else {
        f->flags |= (VFS_TYPE_BLK << 2);
    }

    f->dev = dev;
    f->dev_priv = NULL;

    if (dev->open) {
        return dev->open(dev, f, flags);
    }

    return 0;
}
