#pragma once
#include "../multiboot.h"

extern void (*x86_timer_func) (void);
extern struct multiboot_info *x86_multiboot_info;
