#include "sys/mm.h"
#include "sys/mem.h"
#include "sys/debug.h"
#include "sys/assert.h"

#include "space.h"

#define X86_MM_FLG_PR       (1 << 0)
#define X86_MM_FLG_WR       (1 << 1)
#define X86_MM_FLG_US       (1 << 2)
#define X86_MM_FLG_PS       (1 << 7)

int mm_map_range(mm_space_t pd, uintptr_t start, size_t count, uint32_t flags) {
    if (flags & MM_FLG_PS) {
        panic("Not yet implemented: mm_map_range with MM_FLG_PS\n");
    }

    // Only accept aligned addresses
    assert(!(start & 0xFFF));

    uint32_t dst_flags = (flags & (MM_FLG_WR | MM_FLG_US)) | X86_MM_FLG_PR;
    uint32_t *table_info = (uint32_t *) pd[1023];
    for (size_t i = 0; i < count; ++i) {
        uintptr_t vpage = start + i * 0x1000;
        uint32_t pdi = vpage >> 22;
        uint32_t pti = (vpage >> 12) & 0x3FF;

        assert(!(pd[pdi] & X86_MM_FLG_PS));

        uintptr_t page_phys = mm_alloc_physical_page(0);

        if (pd[pdi] & X86_MM_FLG_PR) {
            assert(table_info[pdi]);
            mm_pagetab_t pt = (mm_pagetab_t) (table_info[pdi] & -0x1000);
            assert((table_info[pdi] & 0xFFF) != 0xFFF);
            ++table_info[pdi];

            kdebug("map %p[%d (%p)] = table %p[%d (%p)] = %p\n", pd, pdi, pdi << 22, pt, pti, vpage, page_phys);
            pt[pti] = page_phys | dst_flags;
        } else {
            uintptr_t pt_phys;
            mm_pagetab_t pt = (mm_pagetab_t) x86_page_pool_allocate(&pt_phys);
            assert((uintptr_t) pt != MM_NADDR);
            table_info[pdi] = (uintptr_t) pt | 1;

            kdebug("map %p[%d (%p)] = table %p\n", pd, pdi, pdi << 22, pt);
            pd[pdi] = pt_phys | (X86_MM_FLG_PR | X86_MM_FLG_WR | X86_MM_FLG_US);

            kdebug("map %p[%d (%p)] = table %p[%d (%p)] = %p\n", pd, pdi, pdi << 22, pt, pti, vpage, page_phys);
            pt[pti] = page_phys | dst_flags;
        }
    }

    return 0;
}

int mm_map_range_pages(mm_space_t pd, uintptr_t start, uintptr_t *pages, size_t count, uint32_t flags) {
    assert(!(start & 0xFFF));

    if (flags & MM_FLG_PS) {
        uint32_t dst_flags = (flags & (MM_FLG_WR | MM_FLG_US)) | X86_MM_FLG_PS | X86_MM_FLG_PR;
        for (int i = 0; i < count; ++i) {
            assert(!(pd[(start >> 22) + i] & X86_MM_FLG_PR));
            kdebug("map %p[%d (%p)] = %p\n", pd, (start >> 22) + i, start + (i << 22), pages[i]);
            pd[(start >> 22) + i] = pages[i] | dst_flags;
        }
        return 0;
    } else {
        uint32_t dst_flags = (flags & (MM_FLG_WR | MM_FLG_US)) | X86_MM_FLG_PR;
        uint32_t *table_info = (uint32_t *) pd[1023];
        for (int i = 0; i < count; ++i) {
            uintptr_t vpage = start + i * 0x1000;
            uint32_t pdi = vpage >> 22;
            uint32_t pti = (vpage >> 12) & 0x3FF;

            uint32_t pde = pd[pdi];

            assert(!(pde & X86_MM_FLG_PS));

            if (pde & X86_MM_FLG_PR) {
                // Lookup table address in table info structure
                assert(table_info[pdi]);
                mm_pagetab_t pt = (mm_pagetab_t) (table_info[pdi] & -0x1000);
                assert((table_info[pdi] & 0xFFF) != 0xFFF);
                ++table_info[pdi];

                kdebug("map %p[%d (%p)] = table %p[%d (%p)] = %p\n", pd, pdi, pdi << 22, pt, pti, vpage, pages[i]);
                pt[pti] = pages[i] | dst_flags;
            } else {
                uintptr_t phys;
                mm_pagetab_t pt = (mm_pagetab_t) x86_page_pool_allocate(&phys);
                assert((uintptr_t) pt != MM_NADDR);
                table_info[pdi] = (uintptr_t) pt | 1;

                kdebug("map %p[%d (%p)] = table %p\n", pd, pdi, pdi << 22, pt);
                pd[pdi] = phys | (X86_MM_FLG_PR | X86_MM_FLG_WR | X86_MM_FLG_US);

                kdebug("map %p[%d (%p)] = table %p[%d (%p)] = %p\n", pd, pdi, pdi << 22, pt, pti, vpage, pages[i]);
                pt[pti] = pages[i] | dst_flags;
            }
        }

        return 0;
    }

    return -1;
}

int mm_map_range_linear(mm_space_t pd, uintptr_t start, uintptr_t pstart, size_t count, uint32_t flags) {
    assert(!(start & 0xFFF));
    assert(!(pstart & 0xFFF));

    if (flags & MM_FLG_PS) {
        start &= -0x400000;

        for (int i = 0; i < count; ++i) {
            uintptr_t ppage = pstart + i * 0x400000;
            if (mm_map_range_pages(pd, start + i * 0x400000, &ppage, 1, flags) != 0) {
                // TODO: free previously mapped pages
                return -1;
            }
        }
    } else {
        start &= -0x1000;

        for (int i = 0; i < count; ++i) {
            uintptr_t ppage = pstart + i * 0x1000;
            if (mm_map_range_pages(pd, start + i * 0x1000, &ppage, 1, flags) != 0) {
                // TODO: free previously mapped pages
                return -1;
            }
        }
    }

    return 0;
}

int mm_umap_range(mm_space_t pd, uintptr_t start, size_t count, uint32_t flags) {
    assert(!(start & 0xFFF));

    if (flags & MM_FLG_PS) {
        for (int i = 0; i < count; ++i) {
            uint32_t pdi = (start >> 22) + i;
            assert(pd[(start >> 22) + i] & (X86_MM_FLG_PR | X86_MM_FLG_PS));
            kdebug("umap %p[%d (%p)]\n", pd, pdi, pdi << 22);

            if (!(flags & MM_FLG_NOPHYS)) {
                mm_free_physical_page(pd[pdi] & -0x400000, MM_FLG_PS);
            }

            pd[pdi] = 0;
        }
    } else {
        uint32_t *table_info = (uint32_t *) pd[1023];
        for (int i = 0; i < count; ++i) {
            uintptr_t vpage = start + i * 0x1000;
            uint32_t pdi = vpage >> 22;
            uint32_t pti = (vpage >> 12) & 0x3FF;

            uint32_t pde = pd[pdi];

            assert(pde & X86_MM_FLG_PR);
            assert(table_info[pdi]);
            mm_pagetab_t pt = (mm_pagetab_t) (table_info[pdi] & -0x1000);

            if (!(flags & MM_FLG_NOPHYS)) {
                mm_free_physical_page(pt[pti] & -0x1000, 0);
            }
            assert((table_info[pdi] & 0xFFF) != 0);
            kdebug("umap %p[%d (%p)] = table %p[%d (%p)]\n", pd, pdi, pdi << 22, pt, pti, vpage);
            pt[pti] = 0;
            if (((--table_info[pdi]) & 0xFFF) == 0) {
                kdebug("umap table %p[%d (%p)] = %p\n", pd, pdi, pdi << 22, pt);
                pd[pdi] = 0;
                table_info[pdi] = 0;
                x86_page_pool_free((uintptr_t) pt);
            }
        }
    }
    return -1;
}

////

uintptr_t mm_translate(mm_space_t pd, uintptr_t addr, uint32_t *rflags) {
    uint32_t pdi = addr >> 22;
    uint32_t pti = (addr >> 12) & 0x3FF;

    if (!(pd[pdi] & X86_MM_FLG_PR)) {
        return MM_NADDR;
    }

    if (pd[pdi] & X86_MM_FLG_PS) {
        *rflags = (pd[pdi] & (MM_FLG_US | MM_FLG_WR)) | MM_FLG_PS;
        return (pd[pdi] & -0x400000) | (addr & 0x3FFFFF);
    }

    uint32_t *table_info = (uint32_t *) pd[1023];
    assert(table_info[pdi]);
    mm_pagetab_t pt = (mm_pagetab_t) (table_info[pdi] & -0x1000);

    if (!(pt[pti] & X86_MM_FLG_PR)) {
        return MM_NADDR;
    }

    *rflags = (pt[pti] & (MM_FLG_US | MM_FLG_WR));
    return (pt[pti] & -0x1000) | (addr & 0xFFF);
}

void mm_dump_map(int level, mm_space_t pd) {
    kprint(level, "---- Address space dump ----\n");
    kprint(level, "L0 @ %p\n", pd);
    uint32_t *table_info = (uint32_t *) pd[1023];
    for (uint32_t pdi = 0; pdi < 1023; ++pdi) {
        if (pd[pdi] & X86_MM_FLG_PR) {
            if (pd[pdi] & X86_MM_FLG_PS) {
                kprint(level, "\t%p .. %p -> %p\n", pdi << 22, (pdi << 22) + 0x400000, pd[pdi] & -0x400000);
            } else {
                kprint(level, "\t%p .. %p -> L1 %p\n", pdi << 22, (pdi << 22) + 0x400000, pd[pdi] & -0x1000);
                mm_pagetab_t pt = (mm_pagetab_t) (table_info[pdi] & -0x1000);
                for (uint32_t pti = 0; pti < 1024; ++pti) {
                    if (pt[pti] & X86_MM_FLG_PR) {
                        kprint(level, "\t\t%p .. %p -> %p\n", (pdi << 22) | (pti << 12), ((pdi << 22) | (pti << 12)) + 0x1000, pt[pti] & -0x1000);
                    }
                }
            }
        }
    }
}
