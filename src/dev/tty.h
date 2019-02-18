#pragma once
#include "sys/dev.h"

dev_t *tty_get(int n);

void tty_init(void);
