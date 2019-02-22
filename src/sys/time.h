#pragma once
#include <stdint.h>

// Desired resolution for system time
#define SYSTICK_DES_RES     1000000

// TODO: initially fetch this systime from RTC chip or something
typedef uint64_t systick_t;
extern volatile systick_t systime;
