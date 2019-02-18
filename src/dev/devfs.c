#include "devfs.h"

static struct devfs {
    vfs_t fs;
} devfs;

vfs_t *vfs_devfs;

void devfs_init(void) {
    vfs_init(&devfs.fs);
    vfs_devfs = (vfs_t *) &devfs;
}
