#include "devfs.h"
#include "sys/debug.h"
#include "sys/dev.h"
#include "sys/vfs.h"
#include "sys/heap.h"
#include "sys/assert.h"
#include "sys/mem.h"
#include "sys/string.h"
#include "dev/tty.h"
#include "dev/initrd.h"
#include "dev/pseudo.h"

struct devfs_node {
    dev_t *dev;
    uintptr_t dev_param;
    char name[256];
    struct devfs_node *next;
};

static struct devfs {
    vfs_t fs;
    struct devfs_node *nodes;
    struct devfs_node *last;
} devfs;

vfs_t *vfs_devfs;

static int devfs_open(vfs_t *fs, vfs_file_t *f, const char *path, uint32_t flags) {
    for (struct devfs_node *node = devfs.nodes; node; node = node->next) {
        if (!strcmp(node->name, path)) {
            return dev_open(node->dev, f, flags);
        }
    }
    return -1;
}

static int devfs_fact(vfs_t *fs, vfs_mount_t *mnt, const char *path, uint32_t act, ...) {
    va_list args;
    va_start(args, act);
    int res = -1;

    switch (act) {
    case VFS_FACT_BLKDEV:
        for (const struct devfs_node *it = devfs.nodes; it; it = it->next) {
            if (!(it->dev->flags & DEV_FLG_CHR) && !strcmp(it->name, path)) {
                *va_arg(args, dev_t **) = it->dev;
                res = 0;
                break;
            }
        }
        break;
    }

    va_end(args);

    return res;
}

static int devfs_opendir(vfs_t *fs, vfs_mount_t *mnt, vfs_dir_t *dir, const char *path, uint32_t flags) {
    if (path[0]) {
        return -1;
    }

    // Make dir point to first entry
    dir->dev_priv = devfs.nodes;

    return 0;
}

static int devfs_readdir(vfs_t *fs, vfs_dir_t *dir, vfs_dirent_t *ent, uint32_t flags) {
    if (!dir->dev_priv) {
        return -1;
    }

    struct devfs_node *node = (struct devfs_node *) dir->dev_priv;
    strcpy(ent->name, node->name);

    assert(node->dev);
    if (node->dev->flags & DEV_FLG_CHR) {
        ent->flags = VFS_TYPE_CHR << 2;
    } else {
        ent->flags = VFS_TYPE_BLK << 2;
    }

    dir->dev_priv = node->next;

    return 0;
}

void devfs_init(void) {
    devfs.nodes = NULL;
    devfs.last = NULL;

    vfs_init(&devfs.fs);

    devfs.fs.open = devfs_open;
    devfs.fs.fact = devfs_fact;

    devfs.fs.opendir = devfs_opendir;
    devfs.fs.readdir = devfs_readdir;

    vfs_devfs = (vfs_t *) &devfs;

    dev_pseudo_setup();
}

void devfs_add(dev_t *dev, const char *p, uintptr_t param) {
    struct devfs_node *node = (struct devfs_node *) heap_alloc(sizeof(struct devfs_node));

    node->dev = dev;
    node->dev_param = param;
    strcpy(node->name, p);

    if (devfs.last) {
        devfs.last->next = node;
    } else {
        devfs.nodes = node;
    }

    devfs.last = node;
}

void devfs_populate(void) {
    char last_ram = '0';
    char name[5] = {0};
    strcpy(name, "ram");
    if (dev_initrd) {
        name[3] = last_ram++;
        devfs_add(dev_initrd, name, 0);
    }

    devfs_add(dev_null, "null", 0);
    devfs_add(dev_zero, "zero", 0);

    devfs_add(tty_get(0), "tty0", 0);
}
