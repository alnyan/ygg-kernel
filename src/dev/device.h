#pragma once
#include <stdint.h>

typedef int ssize_t;
typedef struct device dev_t;

typedef ssize_t (*dev_write_func)(dev_t *, void *, const void *, size_t);
typedef ssize_t (*dev_read_func)(dev_t *, void *, void *, size_t);

#define DEV_FLG_READ        (1 << 0)
#define DEV_FLG_WRITE       (1 << 1)
#define DEV_FLG_RDAS        (1 << 2)
#define DEV_FLG_WRAS        (1 << 3)

struct device {
    uint32_t flags;
    dev_write_func write;
    dev_read_func read;
};

