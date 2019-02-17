#pragma once
#include "panic.h"

#define assert_strcond(c) #c
#define assert(cond) do { \
                        if (!(cond)) { \
                            panic("Assertion failed: " assert_strcond(cond) "\n"); \
                        } \
                     } while (0)
