#include "devfs.h"
#include "sys/debug.h"
#include "sys/mem.h"
#include "sys/string.h"

// /dev/zero:
//  read - provides exactly len zero bytes
//  write - does nothing
#define DEVFS_PRIV_ZERO     0x0
// /dev/null:
//  read - returns -1 (EOF)
//  write - does nothing
#define DEVFS_PRIV_NULL     0x1

static struct devfs {
    vfs_t fs;
} devfs;

vfs_t *vfs_devfs;

static int devfs_open(vfs_t *fs, vfs_file_t *f, const char *path, uint32_t flags) {
    if (!strcmp(path, "zero")) {
        f->fs_priv = (void *) DEVFS_PRIV_ZERO;
        f->dev_priv = NULL;

        return 0;
    }

    if (!strcmp(path, "null")) {
        f->fs_priv = (void *) DEVFS_PRIV_NULL;
        f->dev_priv = NULL;

        return 0;
    }

    return -1;
}

static ssize_t devfs_read(vfs_t *fs, vfs_file_t *f, void *buf, size_t len, uint32_t flags) {
    switch ((uint32_t) f->fs_priv) {
    case DEVFS_PRIV_ZERO:
        memset(buf, 0, len);
        return len;
    case DEVFS_PRIV_NULL:
        return -1;
    default:
        return -1;
    }
}

static ssize_t devfs_write(vfs_t *fs, vfs_file_t *f, const void *buf, size_t len, uint32_t flags) {
    switch ((uint32_t) f->fs_priv) {
    case DEVFS_PRIV_ZERO:
    case DEVFS_PRIV_NULL:
        return len;
    default:
        return -1;
    }
}

void devfs_init(void) {
    vfs_init(&devfs.fs);

    devfs.fs.open = devfs_open;
    devfs.fs.read = devfs_read;
    devfs.fs.write = devfs_write;

    vfs_devfs = (vfs_t *) &devfs;
}
