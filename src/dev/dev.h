#pragma once
#include <stdint.h>
#include <stddef.h>
#include "sys/task.h"

typedef struct ioman_op ioman_op_t;

#define DEV_TYPE(t)         ((t) & 0x1)
#define DEV_GET_TYPE(d)     ((d)->flags & 0x1)
#define DEV_TYPE_BLK        0
#define DEV_TYPE_CHR        1
#define DEV_FLG_MEMR        (1 << 31)

////

typedef int ssize_t;
typedef struct dev dev_t;

////

typedef int (*dev_would_block_func) (dev_t *, uintptr_t, size_t, int);
typedef int (*dev_read_func) (dev_t *, ioman_op_t *);
typedef int (*dev_write_func) (dev_t *, ioman_op_t *);
typedef ssize_t (*dev_write_imm_func) (dev_t *, const void *, uintptr_t, size_t);
typedef ssize_t (*dev_read_imm_func) (dev_t *, void *, uintptr_t, size_t);
typedef uintptr_t (*dev_getm) (dev_t *, uintptr_t, size_t);

////

struct dev {
    uint32_t flags;
    ioman_op_t *pending;

    dev_would_block_func would_block;

    dev_read_func read;
    dev_write_func write;

    dev_write_imm_func write_imm;
    dev_read_imm_func read_imm;

    dev_getm getm;
};

////

void dev_init(dev_t *dev, int type, uint32_t flags);
