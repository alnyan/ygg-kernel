#pragma once
#include "dev.h"

extern dev_t *dev_initrd;

void initrd_init(uintptr_t base);
