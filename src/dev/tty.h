#pragma once
#include "dev.h"

extern dev_t *dev_tty;

void tty_init(void);
void tty_type(char c);
