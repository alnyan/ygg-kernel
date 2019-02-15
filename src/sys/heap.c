#include "heap.h"
#include <stddef.h>
#include "sys/debug.h"
#include "sys/panic.h"

struct heap_block {
    uint32_t flags;
    size_t size;
    struct heap_block *next;
} __attribute__((packed));

static struct heap_block *s_heap_regions[16];

void heap_init(void) {
    memset(s_heap_regions, 0, sizeof(s_heap_regions));
}

void heap_add_region(uintptr_t a, uintptr_t b) {
    size_t sz = b - a - sizeof(struct heap_block);
    struct heap_block *heap = (struct heap_block *) a;
    heap->flags = HEAP_MAGIC | HEAP_FLG_BEG;
    heap->size = sz;
    heap->next = NULL;
    for (int i = 0; i < sizeof(s_heap_regions) / sizeof(s_heap_regions[0]); ++i) {
        if (!s_heap_regions[i]) {
            s_heap_regions[i] = heap;
            debug("Added %uK to heap\n", sz / 1024);
            return;
        }
    }

    panic("No free heap regions left to allocate\n");
}
