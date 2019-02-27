#pragma once
#include <stdint.h>
#include <stddef.h>

typedef int ssize_t;

#define HEAP_MAGIC          0xDEADF00D
#define HEAP_FLG_USED       (1 << 4)
#define HEAP_FLG_EXCLUDE    (1 << 5)

struct heap_stat {
    size_t free;
    size_t used;
    size_t total;
    size_t blocks;
    size_t frags;
#ifdef ENABLE_HEAP_ALLOC_COUNT
    size_t allocs;
    size_t frees;
    size_t alloc_bytes;
    size_t free_bytes;
#endif
};

void heap_init(void);
void heap_add_region(uintptr_t start, uintptr_t end);
void heap_remove_region(uintptr_t start, size_t size);

void heap_stat(struct heap_stat *st);
void heap_dump(void);

void *heap_alloc(size_t count);
void *heap_realloc(void *ptr, size_t newsz);
void heap_free(void *ptr);
