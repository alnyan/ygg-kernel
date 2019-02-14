#pragma once
#include "regs.h"

typedef struct {
    x86_gp_regs_t gp;
    uint32_t int_no;
    uint32_t err_code;
    x86_iret_regs_t iret;
} x86_int_regs_t;

void ints_init(void);

