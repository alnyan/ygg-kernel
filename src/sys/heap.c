#include "heap.h"
#include <stddef.h>
#include "sys/mm.h"
#include "sys/mem.h"
#include "sys/debug.h"
#include "sys/panic.h"

#define HEAP_MAX        16
#define HEAP_DATA(b)    (((uintptr_t) (b)) + sizeof(struct heap_block))

struct heap_block {
    uint32_t flags;
    size_t size;
    struct heap_block *prev;
    struct heap_block *next;
} __attribute__((packed));

static struct heap_block *s_heap_regions[HEAP_MAX];

void heap_init(void) {
    memset(s_heap_regions, 0, sizeof(s_heap_regions));
}

void heap_add_region(uintptr_t a, uintptr_t b) {
    a = MM_ALIGN_UP(a, 8);
    size_t sz = b - a - sizeof(struct heap_block);
    struct heap_block *heap = (struct heap_block *) a;
    heap->flags = HEAP_MAGIC;
    heap->size = sz;
    heap->prev = NULL;
    for (int i = 0; i < HEAP_MAX; ++i) {
        if (!s_heap_regions[i]) {
            s_heap_regions[i] = heap;
            debug("Added %uK to heap\n", sz / 1024);
            return;
        }
    }

    panic("No free heap regions left to allocate\n");
}

static void *heap_alloc_single(struct heap_block *begin, size_t count) {
    struct heap_block *it;

    for (it = begin; it; it = it->next) {
        if (it->flags & HEAP_FLG_USED) {
            continue;
        }

        if (it->size < count) {
            continue;
        }

        /*
         * Case 1: count <= it->size < count + sizeof(struct heap_block)
         * We can only fit our allocation, but no new block will be created
         */
        if (it->size >= count && it->size < count + sizeof(struct heap_block)) {
            it->flags |= HEAP_FLG_USED;
            it->size = count;
            return HEAP_DATA(it);
        }

        /*
         * Case 2: count >= it->size + sizeof(struct heap_block)
         * Can create next header
         */
        struct heap_block *newb = HEAP_DATA(it) + count;

        // Link
        newb->next = it->next;
        newb->prev = it;
        it->next = newb;

        newb->size = it->size - count - sizeof(struct heap_block);
        it->size = count;

        it->flags |= HEAP_FLG_USED;
        newb->flags = HEAP_MAGIC;

        return HEAP_DATA(it);
    }

    return NULL;
}

static int heap_free_single(struct heap_block *begin, void *ptr) {
    struct heap_block *it = (struct heap_block *) (((uintptr_t) ptr) - sizeof(struct heap_block));

    if (it->flags & HEAP_MAGIC == HEAP_MAGIC && (it->flags & HEAP_FLG_USED)) {
        it->flags ^= HEAP_FLG_USED;

        if (it->next && !(it->next->flags & HEAP_FLG_USED)) {
            it->size += it->next->size + sizeof(struct heap_block);
            if (it->next->next) {
                it->next->next->prev = it;
            }
            it->next->flags &= ~HEAP_MAGIC;
            it->next = it->next->next;
        }

        if (it->prev && !(it->prev->flags & HEAP_FLG_USED)) {
            it = it->prev;

            it->size += it->next->size + sizeof(struct heap_block);
            it->next->flags &= ~HEAP_MAGIC;
            it->next = it->next->next;
            if (it->next) {
                it->next->prev = it;
            }
        }
        return 0;
    } else {
        return -1;
    }
}

void *heap_alloc(size_t count) {
    debug("heap_alloc %u\n", count);
    void *res;
    // TODO: expanding heap
    for (int i = 0; i < HEAP_MAX; ++i) {
        if (!s_heap_regions[i]) {
            return NULL;
        }

        if ((res = heap_alloc_single(s_heap_regions[i], count))) {
            return res;
        }
    }
    return NULL;
}

void *heap_realloc(void *ptr, size_t count) {
    struct heap_block *it = (struct heap_block *) (ptr - sizeof(struct heap_block));

    if (it->flags & HEAP_MAGIC != HEAP_MAGIC) {
        panic("Heap: invalid pointer\n");
    }

    ssize_t diff = count - it->size;

    if (diff < 0) {
        // Shrinking
        if (-diff >= sizeof(struct heap_block)) {
            // Just create a new free block
            debug("Shrinking!\n");

            struct heap_block *newb = HEAP_DATA(it) + count;
            newb->size = -diff - sizeof(struct heap_block);
            newb->prev = it;
            newb->flags = HEAP_MAGIC;
            it->size = count;

            if (it->next) {
                newb->next = it->next;
                it->next = newb;

                if (newb->next) {
                    newb->next->prev = newb;
                }

                // May join with next block
                if (!(newb->next->flags & HEAP_FLG_USED)) {
                    debug("Joining\n");
                    struct heap_block *next = newb->next;
                    debug("%p->next = %p\n", newb, next);

                    newb->size += sizeof(struct heap_block) + next->size;
                    newb->next = next->next;
                    if (next->next) {
                        next->next->prev = newb;
                    }

                    next->flags ^= HEAP_MAGIC;
                }
            } else {
                newb->next = NULL;
            }

            return HEAP_DATA(it);
        } else {
            return HEAP_DATA(it);
        }
    } else {
        // Expanding
        // Check if we can expand using next free block
        if (it->next && !(it->next->flags & HEAP_FLG_USED)) {
            struct heap_block *next = it->next;

            if (diff >= next->size && diff - sizeof(struct heap_block) <= next->size) {
                // No need to insert new blocks
                it->next = next->next;
                if (it->next) {
                    it->next->prev = it;
                }
                next->flags ^= HEAP_MAGIC;
                it->size = count;

                return HEAP_DATA(it);
            }

            if (diff < next->size) {
                // Which means we can insert a block
                struct heap_block *newb = HEAP_DATA(it) + count;
                newb->prev = it;
                newb->next = next->next;
                if (newb->next) {
                    newb->next->prev = newb;
                }
                it->next = newb;
                next->flags ^= HEAP_MAGIC;
                newb->size = next->size - diff;
                it->size = count;
                newb->flags = HEAP_MAGIC;

                return HEAP_DATA(it);
            }

            // Otherwise, the block cannot fit our size
            panic("NYI2\n");
        } else {
            // Need to deallocate the current block, then allocate a new one and copy the data
            panic("NYI\n");
        }
    }
}

void heap_free(void *ptr) {
    debug("heap_free %p\n", ptr);

    if (!ptr) {
        return;
    }

    for (int i = 0; i < HEAP_MAX; ++i) {
        if (!s_heap_regions[i]) {
            panic("Heap: invalid pointer\n");
        }

        if ((uintptr_t) ptr > (uintptr_t) s_heap_regions[i] && heap_free_single(s_heap_regions[i],
                    ptr) == 0) {
            return;
        }
    }

    panic("Heap: invalid pointer\n");
}

void heap_stat(struct heap_stat *st) {
    memset(st, 0, sizeof(struct heap_stat));

    for (int i = 0; i < HEAP_MAX; ++i) {
        if (!s_heap_regions[i]) {
            break;
        }

        struct heap_block *it;
        ++st->frags;

        for (it = s_heap_regions[i]; it; it = it->next) {
            if (it->flags & HEAP_FLG_USED) {
                st->used += it->size;
            } else {
                st->free += it->size;
            }

            ++st->blocks;
        }
    }

    st->total = st->used + st->free + st->blocks * sizeof(struct heap_block);
}

void heap_dump(void) {
    for (int i = 0; i < HEAP_MAX; ++i) {
        if (!s_heap_regions[i]) {
            break;
        }

        debug("Heap region #%d:\n", i);

        struct heap_block *it;
        int j;

        for (it = s_heap_regions[i], j = 0; it; it = it->next, ++j) {
            debug(" [%d] %p %c%c%c%c %uB (%uB)\n",
                    j,
                    HEAP_DATA(it),
                    (it->flags & HEAP_FLG_USED) ? 'a' : '-',
                    (it->prev) ? 'p' : '-',
                    (it->next) ? 'n' : '-',
                    (it->flags & HEAP_MAGIC == HEAP_MAGIC) ? '-' : '!',
                    it->size,
                    it->size + sizeof(struct heap_block));
        }
    }
}
