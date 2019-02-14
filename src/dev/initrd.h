#pragma once
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uintptr_t base;
    size_t len;
} dev_initrd_t;

void initrd_init(dev_initrd_t *dev, uintptr_t base, size_t size);
uintptr_t initrd_find_file(const dev_initrd_t *dev, const char *filename);
