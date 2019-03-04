#include "initrd.h"
#include "sys/string.h"
#include "sys/assert.h"
#include "sys/debug.h"
#include "sys/mem.h"
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

static uintptr_t initrd_find_file(uintptr_t base, const char *name) {
    size_t filesz;
    int zb = 0;
    tar_t *it = (tar_t *) base;

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
                return (uintptr_t) it;
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

static int initramfs_check_path_match(const char *p0, const char *p1) {
    int d0 = 0, d1 = 0;
    if (strncmp(p0, p1, strlen(p0))) {
        return -1;
    }

    if (*p0) {
        ++d0;
    }

    for (const char *p = p0; *p; ++p) {
        if (*p == '/') {
            ++d0;
        }
    }
    for (const char *p = p1; *p; ++p) {
        if (*p == '/' && *(p + 1)) {
            ++d1;
        }
    }

    return (d0 == d1) ? 0 : -1;
}

// XXX: make sure this code filters entries by level and dir name
static int initramfs_readdir(vfs_t *fs, vfs_dir_t *dir, vfs_dirent_t *ent, uint32_t flags) {
    uintptr_t pos = (uintptr_t) dir->dev_priv;

    if (pos == MM_NADDR) {
        return -1;
    }

    tar_t *hdr = (tar_t *) pos;
    int is_ustar = tar_is_ustar(hdr);
    int d = 0;
    for (const char *p = dir->path; *p; ++p) {
        if (*p == '/') {
            ++d;
        }
    }

    while (initramfs_check_path_match(dir->path, hdr->name)) {
        if (tar_type(hdr, is_ustar) == TAR_FILE) {
            size_t entsiz = tar_oct2u32(hdr->size, 12);
            hdr = &hdr[1 + MM_ALIGN_UP(entsiz, 512) / 512];
        } else {
            hdr = &hdr[1];
        }

        if (!hdr->name[0]) {
            dir->dev_priv = (void *) MM_NADDR;
            return -1;
        }
    }

    strcpy(ent->d_name, hdr->name + (*dir->path ? (strlen(dir->path) + 1) : 0));

    if (tar_type(hdr, is_ustar) == TAR_FILE) {
        ent->d_type = VFS_DT_REG;

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
        ent->d_type = VFS_DT_DIR;
        hdr = &hdr[1];
    }

    dir->dev_priv = hdr;

    return 0;
}

static int initramfs_open(vfs_t *fs, vfs_file_t *f, const char *path, uint32_t flags) {
    // Lookup the file
    // TODO: rewrite using mount instead of fs
    uintptr_t file_loc = initrd_find_file(initrd.base, path);

    if (file_loc == MM_NADDR) {
        return -1;
    }

    tar_t *tar = (tar_t *) file_loc;

    // Setup tracking
    f->pos0 = (uintptr_t) &tar[1];
    f->pos1 = tar_oct2u32(tar->size, 12);

    return 0;
}

static ssize_t initramfs_read(vfs_t *fs, vfs_file_t *f, void *data, size_t len, uint32_t flags) {
    size_t l = len > f->pos1 ? f->pos1 : len;
    memcpy(data, (const void *) f->pos0, l);
    f->pos1 -= l;
    f->pos0 += l;

    return l;
}

static int initramfs_fact(vfs_t *fs, vfs_mount_t *mnt, const char *path, uint32_t action, ...) {
    int res = -1;
    va_list args;
    va_start(args, action);

    switch (action) {
    case VFS_FACT_GETM:
        {
            uintptr_t f = initrd_find_file(initrd.base, path);

            if (f == MM_NADDR) {
                kdebug("Not found\n");
                break;
            }

            *va_arg(args, uintptr_t *) = f + 512;
            res = 0;
        }
        break;
    }

    va_end(args);
    return res;
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
    initramfs.fact = initramfs_fact;
    initramfs.read = initramfs_read;
    initramfs.open = initramfs_open;

    vfs_initramfs = &initramfs;
}
