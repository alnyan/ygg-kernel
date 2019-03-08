#pragma once
#include <stdint.h>
#include <stddef.h>
#include "sys/task.h"

#define DEV_TYPE(t)         ((t) & 0x1)
#define DEV_GET_TYPE(d)     ((d)->flags & 0x1)
#define DEV_TYPE_BLK        0
#define DEV_TYPE_CHR        1

////

typedef int ssize_t;
typedef struct dev dev_t;

////

typedef int (*dev_read_func) (dev_t *, task_t *, void *, uintptr_t, size_t, ssize_t *);

////

struct dev {
    uint32_t flags;

    dev_read_func read;
};

////

void dev_init(dev_t *dev, int type);
