#include "vfs.h"
#include "sys/dev.h"
#include "sys/assert.h"
#include "sys/mem.h"
#include "sys/debug.h"
#include "sys/string.h"
#include "sys/heap.h"
#include "sys/mm.h"
#include "sys/task.h"

// TODO: better data structure
static vfs_mount_t vfs_mounts[16];

////

void vfs_init(vfs_t *fs) {
    memset(fs, 0, sizeof(vfs_t));
}

////

ssize_t vfs_gets(vfs_file_t *f, char *buf, size_t lim) {
    size_t i = 0;
    ssize_t r;
    char c;

    while (i < (lim - 1)) {
        if (vfs_read(f, &c, 1, &r) != 1) {
            if (i == 0) {
                return -1;
            } else {
                break;
            }
        }

        if (c == '\n') {
            break;
        }

        buf[i++] = c;
    }

    buf[i] = 0;
    return i;
}

////

vfs_file_t *vfs_open(const char *path, uint32_t flags) {
    vfs_file_t *ret = NULL;
    const char *rel = NULL;
    vfs_mount_t *mount = NULL;

    if (vfs_lookup_file(path, &mount, &rel) != 0) {
        return ret;
    }

    assert(mount->fs && mount->fs->open);

    ret = heap_alloc(sizeof(vfs_file_t));
    memset(ret, 0, sizeof(vfs_file_t));

    ret->fs = mount->fs;
    // Copy RD/WR flags
    ret->flags |= (flags & 0x3);

    if (mount->fs->open(mount->fs, ret, rel, flags) != 0) {
        heap_free(ret);
        return NULL;
    }

    return ret;
}

void vfs_close(vfs_file_t *f) {
    if (f) {
        assert(f->fs);

        if (f->fs->close) {
            f->fs->close(f->fs, f, 0);
        }

        heap_free(f);
    }
}

ssize_t vfs_read(vfs_file_t *f, void *buf, size_t len, ssize_t *res) {
    assert(f);

    if (!(f->flags & VFS_FLG_RD)) {
        return -1;
    }

    if (f->op_buf) {
        panic("The file is locked by some other operation\n");
    }

    f->op_buf = buf;
    f->op_res = res;
    *f->op_res = 0;
    f->op_len = len;
    f->op_type = (f->flags & 0x3);

    ssize_t r = -1;

    switch ((f->flags >> 2) & 0x7) {
    // FILE
    case VFS_TYPE_REG:
        assert(f->fs);
        assert(f->fs->read);

        r = f->fs->read(f->fs, f, buf, len, 0);
        break;
    // BLK & CHR
    case VFS_TYPE_BLK:
    case VFS_TYPE_CHR:
        assert(f->dev);
        assert(f->dev->read);

        r = f->dev->read(f->dev, f, buf, len, 0);
        break;
    default:
        panic("Unsupported descriptor type\n");
    }

    // If the operation is already completed (does not require an IRQ) we can unlock the file
    if (r != VFS_READ_ASYNC) {
        f->op_buf = NULL;
    }

    return r;
}

ssize_t vfs_write(vfs_file_t *f, const void *buf, size_t len) {
    assert(f);

    if (!(f->flags & VFS_FLG_WR)) {
        return -1;
    }

    switch ((f->flags >> 2) & 0x7) {
    // FILE
    case VFS_TYPE_REG:
        assert(f->fs);
        assert(f->fs->write);

        return f->fs->write(f->fs, f, buf, len, 0);
    case VFS_TYPE_BLK:
    case VFS_TYPE_CHR:
        assert(f->dev);
        assert(f->dev->write);

        return f->dev->write(f->dev, f, buf, len, 0);
    default:
        panic("Unsupported descriptor type\n");
    }
}

//// fact

uintptr_t vfs_getm(const char *path) {
    vfs_mount_t *mnt;
    const char *rel;
    uintptr_t res;

    if (vfs_lookup_file(path, &mnt, &rel) != 0) {
        return MM_NADDR;
    }

    assert(mnt->fs && mnt->fs->fact);

    if (mnt->fs->fact(mnt->fs, mnt, rel, VFS_FACT_GETM, &res) != 0) {
        return MM_NADDR;
    }

    return res;
}

int vfs_stat(const char *path, struct vfs_stat *st) {
    vfs_mount_t *mnt;
    const char *rel;

    if (vfs_lookup_file(path, &mnt, &rel) != 0) {
        return -1;
    }

    assert(mnt->fs && mnt->fs->fact);

    return mnt->fs->fact(mnt->fs, mnt, rel, VFS_FACT_STAT, st);
}

dev_t *vfs_get_blkdev(const char *path) {
    vfs_mount_t *mnt;
    const char *rel;

    if (vfs_lookup_file(path, &mnt, &rel) != 0) {
        return NULL;
    }

    assert(mnt->fs && mnt->fs->fact);

    dev_t *dev;
    if (mnt->fs->fact(mnt->fs, mnt, rel, VFS_FACT_BLKDEV, &dev) != 0) {
        return NULL;
    }
    return dev;
}

vfs_dir_t *vfs_opendir(const char *path) {
    vfs_mount_t *mnt;
    const char *rel;

    if (vfs_lookup_file(path, &mnt, &rel) != 0) {
        return NULL;
    }

    assert(mnt->fs && mnt->fs->opendir);

    vfs_dir_t *dir = (vfs_dir_t *) heap_alloc(sizeof(vfs_dir_t));

    strcpy(dir->path, rel);
    dir->flags |= VFS_FLG_DIR;
    dir->dev = mnt->srcdev;
    dir->fs = mnt->fs;

    if (mnt->fs->opendir(mnt->fs, mnt, dir, rel, 0) != 0) {
        heap_free(dir);
        return NULL;
    }

    return dir;
}

int vfs_readdir(vfs_dir_t *dir, vfs_dirent_t *ent) {
    assert(dir && dir->fs);
    assert(dir->flags & VFS_FLG_DIR);

    if (!dir->fs->readdir) {
        return -1;
    }

    return dir->fs->readdir(dir->fs, dir, ent, 0);
}

void vfs_closedir(vfs_dir_t *dir) {
    assert(dir && dir->fs);
    assert(dir->flags & VFS_FLG_DIR);

    if (dir->fs->closedir) {
        dir->fs->closedir(dir->fs, dir, 0);
    }

    heap_free(dir);
}

void vfs_dirent_dump(const vfs_dirent_t *ent) {
    const char *p;
    switch (ent->d_type) {
    case VFS_DT_CHR:
        p = "chr";
        break;
    case VFS_DT_BLK:
        p = "blk";
        break;
    case VFS_DT_DIR:
        p = "dir";
        break;
    case VFS_DT_REG:
        p = "reg";
        break;
    }
    kdebug(" %12s %-16s\n", p, ent->d_name);
}

////

static int vfs_components_match(const char *p0, const char *p1) {
    int match = 0;
    const char *e0, *e1;

    if (!strcmp(p1, "/")) {
        return 1;
    }

    if (p0[0] == '/') {
        ++p0;
    }

    if (p1[0] == '/') {
        ++p1;
    }

    while (1) {
        e0 = strchrnul(p0, '/');
        e1 = strchrnul(p1, '/');
        if ((e1 - p1) == (e0 - p0) && !strncmp(p0, p1, e0 - p0)) {
            ++match;
            p0 = e0 + 1;
            p1 = e1 + 1;
        } else {
            break;
        }
    }

    return match;
}

// TODO: rewrite this to handle path components properly
int vfs_lookup_file(const char *path, vfs_mount_t **mount, const char **rel) {
    size_t lmatch = 0;
    int ismatchs = 0;

    for (int i = 0; i < sizeof(vfs_mounts) / sizeof(vfs_mounts[0]); ++i) {
        if (vfs_mounts[i].dst[0]) {
            if (!strcmp(vfs_mounts[i].dst, path)) {
                *rel = path + strlen(path);
                *mount = &vfs_mounts[i];
                lmatch = 10000;
                break;
            }

            int match = vfs_components_match(path, vfs_mounts[i].dst);
            if (match > lmatch || (match >= lmatch && ismatchs)) {
                const char *e = path + 1;
                if (strcmp(vfs_mounts[i].dst, "/")) {
                    for (int i = 0; i < match; ++i) {
                        e = strchr(e, '/');
                    }
                    *rel = e + 1;
                    ismatchs = 0;
                } else {
                    ismatchs = 1;
                    *rel = e;
                }

                *mount = &vfs_mounts[i];
                lmatch = match;
            }
        } else {
            break;
        }
    }

    return lmatch == 0 ? -1 : 0;
}

int vfs_mount(const char *src, const char *dst, vfs_t *fs_type, uint32_t opts) {
    assert(dst);
    assert(fs_type);

    kinfo("mount %s -> %s, fs %p\n", src, dst, fs_type);

    for (int i = 0; i < sizeof(vfs_mounts) / sizeof(vfs_mounts[0]); ++i) {
        if (vfs_mounts[i].dst[0]) {
            if (!strcmp(vfs_mounts[i].dst, dst)) {
                kerror("Mountpoint already exists\n");
                return -1;
            }
        } else {
            memset(&vfs_mounts[i], 0, sizeof(vfs_mount_t));
            if (src) {
                dev_t *blkd = vfs_get_blkdev(src);

                if (!blkd) {
                    kerror("%s is not a block device\n", src);
                    return -1;
                }

                vfs_mounts[i].srcdev = blkd;
                strcpy(vfs_mounts[i].src, src);
            }
            strcpy(vfs_mounts[i].dst, dst);
            vfs_mounts[i].fs = fs_type;

            if (fs_type->mount) {
                int res;

                if ((res = fs_type->mount(fs_type, &vfs_mounts[i], opts)) != 0) {
                    return res;
                }
            }

            return 0;
        }
    }

    return -1;
}

////

#if defined(ENABLE_TASK)
int vfs_send_read_res(vfs_file_t *f, const void *src, size_t count) {
    assert(f && f->op_buf && f->op_res);

    if (*f->op_res == -1) {
        // Buffer was broken by some error
        f->op_buf = NULL;
        return 1;
    }

    size_t sz = count > f->op_len ? f->op_len : count;

    if (f->task) {
        mm_memcpy_kernel_to_user(task_space(f->task), (void *) ((uintptr_t) f->op_buf + *f->op_res), src, sz);
        // task_copy_to_user(f->task, (void *) ((uintptr_t) f->op_buf + *f->op_res), src, sz);
    } else {
        memcpy((void *) ((uintptr_t) f->op_buf + *f->op_res), src, sz);
    }
    *f->op_res += sz;

    if (*f->op_res == f->op_len) {
        f->op_buf = NULL;
        if (f->task) {
            task_nobusy(f->task);
        }
        return 1;
    }

    return 0;
}
#endif
