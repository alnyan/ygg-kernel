#pragma once
#include <stdint.h>
#include <stddef.h>

#define MM_ALIGN_UP(p, a)   (((p) + (a) - 1) & ~((a) - 1))

#define MM_FLG_WR           (1 << 1)
#define MM_FLG_US           (1 << 2)
#define MM_FLG_PS           (1 << 7)

#define MM_FLG_NOPHYS       (1 << 30)
#define MM_FLG_DMA          (1 << 31)

#define MM_FLG_CLONE_KERNEL (1 << 0)
#define MM_FLG_CLONE_USER   (1 << 1)
#define MM_FLG_CLONE_HW     (1 << 2)

#if defined(ARCH_X86)
#include "arch/x86/mm.h"
#endif

void mm_init(void);

uintptr_t mm_map_range(mm_space_t pd, uintptr_t start, size_t count, uint32_t flags);
int mm_map_range_pages(mm_space_t pd, uintptr_t start, uintptr_t *pages, size_t count, uint32_t flags);
int mm_umap_range(mm_space_t pd, uintptr_t start, size_t count, uint32_t flags);
uintptr_t mm_translate(mm_space_t pd, uintptr_t vaddr, uint32_t *rflags);

int mm_memcpy_kernel_to_user(mm_space_t dst_pd, void *dst, const void *src, size_t count);
int mm_memcpy_user_to_kernel(mm_space_t src_pd, void *dst, const void *src, size_t count);
ssize_t mm_strncpy_user_to_kernel(mm_space_t src_pd, void *dst, const void *src, size_t count);

uintptr_t mm_alloc_physical_page(uint32_t flags);
void mm_free_physical_page(uintptr_t page, uint32_t flags);

mm_space_t mm_create_space(uintptr_t *phys);
void mm_destroy_space(mm_space_t pd);
void mm_space_clone(mm_space_t dst, const mm_space_t src, uint32_t flags);
int mm_space_fork(mm_space_t dst, const mm_space_t src, uint32_t flags);
void mm_set(mm_space_t pd);

void mm_dump_stats(int level);
void mm_dump_map(int level, mm_space_t pd);

