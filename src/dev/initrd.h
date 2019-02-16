#pragma once
#include <stddef.h>
#include <stdint.h>
#include "dev/device.h"

typedef struct {
    dev_t device;
    uintptr_t base;
    size_t len;
} dev_initrd_t;

typedef enum {
    TAR_FILE,
    TAR_LINK_HARD,
    TAR_LINK_SYM,
    TAR_CHARDEV,
    TAR_BLKDEV,
    TAR_DIR,
    TAR_FIFO,
    TAR_UNDEF
} tar_type_t;

typedef struct {
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
    char padding[12];
} tar_t;

void initrd_init(dev_initrd_t *dev, uintptr_t base, size_t size);
