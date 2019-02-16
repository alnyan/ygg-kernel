#include "initrd.h"
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
            return TAR_FILE;
        }
    }
}

void initrd_init(dev_initrd_t *dev, uintptr_t addr, size_t len) {
    dev->device.flags = DEV_FLG_READ;
    dev->base = addr;
    dev->len = len;

    int files = 0;
    int zb = 0;
    tar_t *it = (tar_t *) addr;
    uint32_t filesz;
    char siz_buf[11];
    // Print out some stats
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
        ++files;

        int is_ustar = tar_is_ustar(it);
        tar_type_t type = tar_type(it, is_ustar);
        if (type == TAR_FILE) {
            filesz = tar_oct2u32(it->size, 12);
            size_t jmp = MM_ALIGN_UP(filesz, 512) / 512;
            fmtsiz(filesz, siz_buf);

            debug(" %10s %s\n", siz_buf, it->name);
            it = &it[jmp + 1];
        } else {
            debug("        DIR %s\n", it->name);
            it = &it[1];
        }
    }
}

uintptr_t initrd_find_file(const dev_initrd_t *dev, const char *name) {

}
