#include "vfs.h"
#include "sys/dev.h"
#include "sys/assert.h"
#include "sys/mem.h"
#include "sys/debug.h"
#include "sys/string.h"
#include "sys/heap.h"

// TODO: better data structure
static vfs_mount_t vfs_mounts[16];

////

void vfs_init(vfs_t *fs) {
    memset(fs, 0, sizeof(vfs_t));
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

// TODO: allow these operations to be done asynchronously via IRQs
ssize_t vfs_read(vfs_file_t *f, void *buf, size_t len) {
    assert(f);

    if (!(f->flags & VFS_FLG_RD)) {
        return -1;
    }

    switch ((f->flags >> 2) & 0x7) {
    // FILE
    case VFS_TYPE_REG:
        assert(f->fs);
        assert(f->fs->read);

        return f->fs->read(f->fs, f, buf, len, 0);
    // BLK & CHR
    case VFS_TYPE_BLK:
    case VFS_TYPE_CHR:
        assert(f->dev);
        assert(f->dev->read);

        return f->dev->read(f->dev, f, buf, len, 0);
    default:
        panic("Unsupported descriptor type\n");
    }
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

////

int vfs_lookup_file(const char *path, vfs_mount_t **mount, const char **rel) {
    size_t lmatch = 0xFFFFFFFF;

    for (int i = 0; i < sizeof(vfs_mounts) / sizeof(vfs_mounts[0]); ++i) {
        if (vfs_mounts[i].dst[0]) {
            // For example:
            //  mount.dst = "/dev"
            //  path = "/dev/some/path"
            //  Then
            //  *rel = "some/path"
            //  *mount = { devfs, "/dev", ... }
            size_t match = strncmn(vfs_mounts[i].dst, path, sizeof(vfs_mounts[i].dst));

            // Full match
            if (match == strlen(vfs_mounts[i].dst) && (path[match] == '/' || !path[match])) {
                if (match < lmatch) {
                    *rel = &path[(path[match] == '/') ? (match + 1) : match];
                    *mount = &vfs_mounts[i];
                    lmatch = match;
                }
            }
        } else {
            break;
        }
    }

    return lmatch == 0xFFFFFFFF ? -1 : 0;
}

int vfs_mount(const char *src, const char *dst, vfs_t *fs_type, uint32_t opts) {
    assert(dst);
    assert(fs_type);

    debug("mount %s -> %s, fs %p\n", src, dst, fs_type);

    for (int i = 0; i < sizeof(vfs_mounts) / sizeof(vfs_mounts[0]); ++i) {
        if (vfs_mounts[i].dst[0]) {
            if (!strcmp(vfs_mounts[i].dst, dst)) {
                return -1;
            }
        } else {
            memset(&vfs_mounts[i], 0, sizeof(vfs_mount_t));
            if (src) {
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
