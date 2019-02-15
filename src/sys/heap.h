#pragma once
#include <stdint.h>

#define HEAP_MAGIC      0xDEADF00D
#define HEAP_FLG_USED   (1 << 4)
#define HEAP_FLG_BEG    (1 << 5)

void heap_init(void);
void heap_add_region(uintptr_t start, uintptr_t end);
