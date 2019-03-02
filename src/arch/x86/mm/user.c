#include "sys/mm.h"
#include "sys/mem.h"
#include "sys/assert.h"
#include "sys/debug.h"

int mm_memcpy_kernel_to_user(mm_space_t pd, void *dst, const void *src, size_t count) {
    uintptr_t user_begin = (uintptr_t) dst & -0x1000;
    uintptr_t user_end = MM_ALIGN_UP((uintptr_t) dst + count, 0x1000);
    size_t npages = (user_end - user_begin) / 0x1000;
    size_t copied = 0;

    for (size_t i = 0; i < npages; ++i) {
        uintptr_t user_page = user_begin + i * 0x1000;

        uintptr_t user_start = ((uintptr_t) dst > user_page) ? (uintptr_t) dst - user_page : 0;
        uintptr_t user_end = ((uintptr_t) dst + count - user_page);
        if (user_end > 0x1000) {
            user_end = 0x1000;
        }

        memcpy((void *) (user_page + user_start),
               (const void *) ((uintptr_t) src + copied),
               user_end - user_start);

        copied += user_end - user_start;
    }

    return -1;
}
