#pragma once
#include "sys/dev.h"

dev_t *tty_get(int n);

void tty_init(void);
void tty_type(int n, char c);
