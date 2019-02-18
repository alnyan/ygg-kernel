#include "initrd.h"
#include "sys/string.h"
#include "sys/assert.h"
#include "sys/debug.h"
#include "sys/mm.h"

static uint32_t tar_oct2u32(const char *oct, size_t len) {
    uint32_t res = 0;
    for (size_t i = 0; i < len; ++i) {
        if (!oct[i]) {
            continue;
        }
        res <<= 3;
        res |= (oct[i] - '0');
    }
    return res;
}

static int tar_is_ustar(const tar_t *t) {
    return !strncmp(t->magic, "ustar", 5);
}

static tar_type_t tar_type(const tar_t *t, int is_ustar) {
    if (is_ustar) {
        if (!t->typeflag) {
            return TAR_FILE;
        }

        if (t->typeflag - '0' < 7) {
            return t->typeflag - '0';
        }

        return TAR_UNDEF;
    } else {
        for (int i = 0; i < sizeof(t->name); ++i) {
            if (t->name[i] == '/' && (i == sizeof(t->name) - 1 || !t->name[i + 1])) {
                return TAR_DIR;
            }
        }
        return TAR_FILE;
    }
}

static uintptr_t initrd_find_file(const dev_initrd_t *dev, const char *name) {
    size_t filesz;
    int zb = 0;
    tar_t *it = (tar_t *) dev->base;

    while (1) {
        if (it->name[0] == 0) {
            if (zb) {
                break;
            }
            ++zb;
            it = &it[1];
            continue;
        }
        zb = 0;

        int is_ustar = tar_is_ustar(it);
        tar_type_t type = tar_type(it, is_ustar);
        if (type == TAR_FILE) {
            filesz = tar_oct2u32(it->size, 12);
            size_t jmp = MM_ALIGN_UP(filesz, 512) / 512;
            if (!strncmp(it->name, name, sizeof(it->name))) {
                return (uintptr_t) &it[1];
            }
            it = &it[jmp + 1];
        } else {
            it = &it[1];
        }
    }

    return MM_NADDR;
}

////

dev_t *dev_initrd;
vfs_t *vfs_initramfs;

static struct initrd {
    dev_t dev;
    uintptr_t base;
    size_t len;
} initrd;

static vfs_t initramfs;

////

static int initramfs_opendir(vfs_t *fs, vfs_mount_t *mnt, vfs_dir_t *dir, const char *path, uint32_t flags) {
    // TEST
    assert(dev_initrd == mnt->srcdev);
    struct initrd *dev = (struct initrd *) mnt->srcdev;

    // Make dir point to first entry
    dir->dev_priv = (void *) dev->base;

    return 0;
}

// XXX: make sure this code filters entries by level and dir name
static int initramfs_readdir(vfs_t *fs, vfs_dir_t *dir, vfs_dirent_t *ent, uint32_t flags) {
    uintptr_t pos = (uintptr_t) dir->dev_priv;

    if (pos == MM_NADDR) {
        return -1;
    }

    tar_t *hdr = (tar_t *) pos;
    int is_ustar = tar_is_ustar(hdr);

    strcpy(ent->name, hdr->name);

    if (tar_type(hdr, is_ustar) == TAR_FILE) {
        ent->flags = VFS_TYPE_REG << 2;

        size_t entsiz = tar_oct2u32(hdr->size, 12);
        hdr = &hdr[1 + MM_ALIGN_UP(entsiz, 512) / 512];

        if (!hdr->name[0]) {
            hdr = &hdr[1];
            if (!hdr->name[0]) {
                dir->dev_priv = (void *) MM_NADDR;
                return 0;
            }
        }
    } else {
        ent->flags = VFS_TYPE_DIR << 2;
        hdr = &hdr[1];
    }

    dir->dev_priv = hdr;

    return 0;
}

////

void initrd_init(uintptr_t addr, size_t len) {
    dev_init(&initrd.dev, DEV_FLG_RD);
    initrd.base = addr;
    initrd.len = len;

    dev_initrd = (dev_t *) &initrd;

    // Init initramfs
    vfs_init(&initramfs);

    initramfs.opendir = initramfs_opendir;
    initramfs.readdir = initramfs_readdir;

    vfs_initramfs = &initramfs;
}
