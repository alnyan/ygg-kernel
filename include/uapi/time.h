#pragma once
#include <stdint.h>

struct timespec {
    uint64_t tv_sec;
    uint64_t tv_nsec;
};
