#pragma once
#include "sys/dev.h"
#define TTY_FLG_SER     0xF0000000

dev_t *tty_get(int n);

void tty_init(void);
void tty_type(int n, char c);
