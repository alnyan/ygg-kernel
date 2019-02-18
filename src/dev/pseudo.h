#pragma once
#include "sys/dev.h"

#define DEV_PSEUDO_ZERO 0
#define DEV_PSEUDO_NULL 1

extern dev_t *dev_null;
extern dev_t *dev_zero;

void dev_pseudo_setup(void);
