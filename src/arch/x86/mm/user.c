#include "sys/mm.h"
#include "sys/mem.h"
#include "sys/assert.h"
#include "sys/debug.h"

int mm_memcpy_user_to_kernel(mm_space_t pd, void *dst, const void *src, size_t count) {
    uintptr_t user_begin = (uintptr_t) src & -0x1000;
    uintptr_t user_end = MM_ALIGN_UP((uintptr_t) src + count, 0x1000);
    size_t npages = (user_end - user_begin) / 0x1000;
    size_t copied = 0;

    for (size_t i = 0; i < npages; ++i) {
        uintptr_t user_page = user_begin + i * 0x1000;

        uintptr_t user_start = ((uintptr_t) src > user_page) ? (uintptr_t) src - user_page : 0;
        uintptr_t user_end = ((uintptr_t) src + count - user_page);
        if (user_end > 0x1000) {
            user_end = 0x1000;
        }

        uint32_t user_rflags;
        uintptr_t user_page_phys = mm_translate(pd, user_page, &user_rflags);
        assert(user_page_phys != MM_NADDR);
        assert(!(user_rflags & MM_FLG_PS));

        // Map the page
        mm_map_range_pages(mm_kernel, user_page, &user_page_phys, 1, MM_FLG_WR);

        memcpy((void *) ((uintptr_t) dst + copied),
               (const void *) (user_page + user_start),
               user_end - user_start);

        copied += user_end - user_start;

        mm_umap_range(mm_kernel, user_page, 1, 0);
    }

    return 0;
}

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

        uint32_t user_rflags;
        uintptr_t user_page_phys = mm_translate(pd, user_page, &user_rflags);
        assert(user_page_phys != MM_NADDR);
        assert(!(user_rflags & MM_FLG_PS));

        // Map the page
        mm_map_range_pages(mm_kernel, user_page, &user_page_phys, 1, MM_FLG_WR);

        memcpy((void *) (user_page + user_start),
               (const void *) ((uintptr_t) src + copied),
               user_end - user_start);

        copied += user_end - user_start;

        mm_umap_range(mm_kernel, user_page, 1, 0);
    }

    return 0;
}
