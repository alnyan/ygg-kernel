#pragma once
#include <stdint.h>
#include <stddef.h>

typedef int ssize_t;

#define HEAP_MAGIC          0xDEADF00D
#define HEAP_FLG_USED       (1 << 4)
#define HEAP_FLG_EXCLUDE    (1 << 5)

#define heap_alloc(cnt)     __heap_alloc_trace(cnt, __func__)
#define heap_free(v)        __heap_free(v)

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

void *__heap_alloc(size_t count);
void __heap_free(void *ptr);

void *__heap_alloc_trace(size_t count, const char *f);
