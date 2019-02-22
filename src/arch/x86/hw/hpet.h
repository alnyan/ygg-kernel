#pragma once
#include <stdint.h>
#include "hw.h"

void hpet_timer_func(void);
int hpet_available(void);
void hpet_set_base(uint32_t addr);
int hpet_init(void);
