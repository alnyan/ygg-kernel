#include "tarfs.h"
#include "sys/string.h"
#include "sys/debug.h"
#include "fs/ioman.h"
#include "sys/mm.h"
#include "sys/assert.h"

struct tar {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char __pad[12];
};

static vfs_t tarfs;
vfs_t *vfs_tarfs = &tarfs;

// TODO: header cache (linked list for easier navigation)

static size_t octal_parse(const char *p, size_t l) {
    size_t r = 0;
    size_t i = 0;
    while (p[i] && i < l) {
        r <<= 3;
        r |= p[i] - '0';
        ++i;
    }
    return r;
}

static uintptr_t tarfs_getm(vfs_t *fs, vfs_node_t *n) {
    assert(n && fs->dev && fs->dev->getm);
    return fs->dev->getm(fs->dev, n->fd_reg.inode, n->fd_reg.size);
}

static vfs_node_t *tarfs_find_node(vfs_t *fs, task_t *t, const char *path) {
    if (!strcmp(path, "/")) {
        return tarfs.root_node;
    }

    if (fs->dev->flags & DEV_FLG_MEMR) {
        assert(fs->dev->getm);
        struct tar *hdr = (struct tar *) fs->dev->getm(fs->dev, 0, 512);
        uintptr_t base = (uintptr_t) hdr;

        if (!hdr) {
            return NULL;
        }

        while (1) {
            if (!hdr->name[0]) {
                return NULL;
            }

            size_t size = octal_parse(hdr->size, 12);

            if (!strcmp(path + 1, hdr->name)) {
                // For now, just create a new node
                vfs_node_t *res = vfs_node_create();
                assert(res);
                // TODO: directories
                res->flags = VFS_NODE_TYPE_REG | VFS_NODE_FLG_MEMR;

                res->fd_reg.fs = fs;
                res->fd_reg.inode = ((uintptr_t) hdr - base) + 512;
                res->fd_reg.off = 0;
                res->fd_reg.size = size;

                return res;
            }

            hdr += 1 + MM_ALIGN_UP(size, 512) / 512;
        }
    } else {
        // Read first header
        uintptr_t pos = 0;
        struct tar hdr;

        while (1) {
            kdebug("read initrd %p\n", pos);

            // TODO: if device is a memory device, just ask it for its base pointer
            if (ioman_dev_read(fs->dev, t, &hdr, pos, 512) != 512) {
                return NULL;
            }

            if (!hdr.name[0]) {
                break;
            }

            size_t size = octal_parse(hdr.size, 12);

            if (!strcmp(path + 1, hdr.name)) {
                // For now, just create a new node
                vfs_node_t *res = vfs_node_create();
                // TODO: directories
                res->flags = VFS_NODE_TYPE_REG;

                res->fd_reg.fs = fs;
                res->fd_reg.inode = pos + 512;
                res->fd_reg.off = 0;
                res->fd_reg.size = size;

                return res;
            }

            pos += 512 + MM_ALIGN_UP(size, 512);
        }
    }

    return NULL;
}

static ssize_t tarfs_read(vfs_t *fs, vfs_node_t *node, void *buf, size_t req) {
    // End of file reached
    if (node->fd_reg.off == node->fd_reg.size) {
        return -1;
    }

    size_t siz = req;
    if (siz > node->fd_reg.size - node->fd_reg.off) {
        siz = node->fd_reg.size - node->fd_reg.off;
    }

    // Read from device
    ssize_t r = ioman_dev_read(fs->dev, node->task, buf, node->fd_reg.inode + node->fd_reg.off, siz);

    node->fd_reg.off += siz;

    return r;
}

void tarfs_init(dev_t *dev) {
    vfs_init(vfs_tarfs);

    tarfs.dev = dev;
    tarfs.pos = 0;
    tarfs.root_node = NULL;

    kdebug("tar size %d\n", sizeof(struct tar));

    tarfs.find_node = tarfs_find_node;
    tarfs.getm = tarfs_getm;
    tarfs.read = tarfs_read;
}
