#pragma once
#include "stdlib.h"
#include "unistd.h"

#define assert(x)   if (!(x)) { \
                        puts("Assertion failed: " #x); \
                        abort(); \
                    }
