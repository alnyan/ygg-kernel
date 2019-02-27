#include "console.h"
#include "sys/mem.h"
#include "arch/hw.h"
#include <stdint.h>
#include "sys/debug.h"
#include "sys/panic.h"
#include "sys/assert.h"
#include "hw.h"
#include "sys/mm.h"
#include "../multiboot.h"

#define X86_CON_BASE    (KERNEL_VIRT_BASE + 0xB8000)

#if defined(ENABLE_VESA_FBCON)
#include "vesa/font.h"
#include "vesa/vesa_logo.h"

#define VESA_FB_VIRT    0xFD000000

static uintptr_t vesa_fb_base = 0;
static uint16_t vesa_char_width = 8;
static uint16_t vesa_char_height = 8;
static uint32_t vesa_attrcol[] = {
    0x00000000,
    0x000000AA,
    0x0000AA00,
    0x0000AAAA,
    0x00AA0000,
    0x00AA00AA,
    0x00AA5500,
    0xAAAAAAAA,
    0x00555555,
    0x005555FF,
    0x0055FF55,
    0x0055FFFF,
    0x00FF5555,
    0x00FF55FF,
    0x00FFFF55,
    0x00FFFFFF,
};

struct vesa_mode_info {
    uint16_t attributes;
    uint8_t win_a, win_b;
    uint16_t granularity;
    uint16_t winsize;
    uint16_t seg_a, seg_b;
    uint32_t real_fct_ptr;
    uint16_t pitch;
    uint16_t x_res, y_res;
    uint8_t wchar, ychar, planes, bpp, banks;
    uint8_t mem_model, bank_size, image_pages;
    uint8_t res0;
    uint8_t red_mask, red_pos;
    uint8_t green_mask, green_pos;
    uint8_t blue_mask, blue_pos;
    uint8_t rsv_mask, rsv_pos;
    uint8_t directcolor_attrs;
    uint32_t physbase;
    uint32_t res1;
    uint16_t res2;
} __attribute__((packed));

static struct vesa_mode_info *vesa_mode_info;
#endif

static uint16_t *con_data = (uint16_t *) X86_CON_BASE;
static uint16_t con_width = 80;
static uint16_t con_height = 25;
static uint16_t con_cury = 0;
static uint16_t con_curx = 0;

static void x86_con_cursor(uint16_t row, uint16_t col) {
    uint16_t pos = row * 80 + col;

    outb(0x03D4, 0x0F);
	outb(0x03D5, (uint8_t) (pos & 0xFF));
	outb(0x03D4, 0x0E);
	outb(0x03D5, (uint8_t) ((pos >> 8) & 0xFF));
}

#if defined(ENABLE_VESA_FBCON)
static inline void x86_vesa_set(uint16_t x, uint16_t y, uint32_t p) {
    // TODO: support non-linear modes
    uint32_t *ptr = (uint32_t *) (vesa_fb_base +
                                  y * vesa_mode_info->pitch +
                                  x * (vesa_mode_info->bpp >> 3));
    *ptr = p;
}

static void x86_vesa_draw_char(uint16_t y, uint16_t x, uint16_t c) {
    char ch = c & 0xFF;
    char fg = (c >> 8) & 0xF;
    char bg = (c >> 12) & 0xF;
    if (con_cury == y && con_curx == x) {
        fg ^= bg;
        bg ^= fg;
        fg ^= bg;
    }

    if (ch >= 0x80) {
        ch = 0;
    }

    uint8_t *bitmap = &vesa_font_8x8[ch * 8];

    for (uint16_t j = 0; j < vesa_char_height; ++j) {
        for (uint16_t i = 0; i < vesa_char_width; ++i) {
            if ((bitmap[j] & (1 << i))) {
                x86_vesa_set(x * vesa_char_width + i, y * vesa_char_height + j, vesa_attrcol[(size_t) fg]);
            } else {
                x86_vesa_set(x * vesa_char_width + i, y * vesa_char_height + j, vesa_attrcol[(size_t) bg]);
            }
        }
    }
}
#endif

static void x86_con_scroll(void) {
    for (int i = 0; i < con_height - 1; ++i) {
        for (int j = 0; j < con_width; ++j) {
            con_data[i * con_width + j] = con_data[(i + 1) * con_width + j];
        }
    }
    memsetw(&con_data[con_height * con_width - con_width], 0x0700, con_width);

#if defined(ENABLE_VESA_FBCON)
    if (vesa_fb_base) {
        for (int i = 0; i < con_height; ++i) {
            for (int j = 0; j < con_width; ++j) {
                x86_vesa_draw_char(i, j, con_data[con_width * i + j]);
            }
        }
    }
#endif
}

void x86_con_putc(char c) {
    if (c >= ' ') {
        if (con_curx == con_width - 1) {
            con_curx = 0;
            ++con_cury;
        }

        if (con_cury == con_height - 1) {
            con_cury = con_height - 2;
            x86_con_scroll();
        }

        con_data[con_cury * con_width + con_curx] = 0x0700 | c;
        ++con_curx;

#if defined(ENABLE_VESA_FBCON)
        if (vesa_fb_base) {
            x86_vesa_draw_char(con_cury, con_curx - 1, 0x0700 | c);
            x86_vesa_draw_char(con_cury, con_curx, con_data[con_cury * con_width + con_curx]);
        }
#endif
    } else if (c == '\n') {
#if defined(ENABLE_VESA_FBCON)
        uint16_t ocx = con_curx;
        uint16_t ocy = con_cury;
#endif
        con_curx = 0;
        ++con_cury;
#if defined(ENABLE_VESA_FBCON)
        if (vesa_fb_base) {
            x86_vesa_draw_char(ocy, ocx, con_data[ocy * con_width + ocx]);
        }
#endif

        if (con_cury == con_height - 1) {
            con_cury = con_height - 2;
            x86_con_scroll();
        }
    } else if (c == '\b') {
        // XXX: should cursor move back one line if at beginning?
        if (con_curx) {
            con_data[con_cury * con_width + (--con_curx)] = 0x0700;

#if defined(ENABLE_VESA_FBCON)
            if (vesa_fb_base) {
                x86_vesa_draw_char(con_cury, con_curx + 1, con_data[con_cury * con_width + (con_curx + 1)]);
                x86_vesa_draw_char(con_cury, con_curx, 0x700);
            }
#endif
        }
    } else {
        x86_con_putc('?');
    }

    x86_con_cursor(con_cury, con_curx);
}

void x86_con_init(void) {
#if defined(ENABLE_VESA_FBCON)
    if (x86_multiboot_info->flags & MULTIBOOT_INFO_VBE_INFO) {
        if (x86_multiboot_info->vbe_mode <= 0x1F) {
            // TODO: support modes other than 80x25 text
        } else {
            uint32_t vbe_mode_info_phys = x86_multiboot_info->vbe_mode_info;
            if (vbe_mode_info_phys >= 0x400000) {
                panic("VBE structure remapping is not implemented yet");
            }

            vesa_mode_info = (struct vesa_mode_info *) (vbe_mode_info_phys + KERNEL_VIRT_BASE);

            kdebug("VBE mode is:\n");
            kdebug("%ux%ux%u\n", vesa_mode_info->x_res, vesa_mode_info->y_res, vesa_mode_info->bpp);

            kdebug("Physical base is: %p\n", vesa_mode_info->physbase);

            // Calculate how many pages we want
            uint32_t vesa_fb_size = (uint32_t) vesa_mode_info->y_res * (uint32_t) vesa_mode_info->pitch;
            uint32_t vesa_fb_pages = MM_ALIGN_UP(vesa_fb_size, MM_PAGESZ) / MM_PAGESZ;

            // TODO: support cases when vesa_fb crosses page boundary
            // Video data storage
            for (uint32_t i = 0; i < vesa_fb_pages; ++i) {
                mm_map_page(mm_kernel, VESA_FB_VIRT + i * MM_PAGESZ, vesa_mode_info->physbase + i * MM_PAGESZ, MM_FLG_RW | MM_FLG_HUGE);
            }
            // Text buffer

            vesa_fb_base = VESA_FB_VIRT | (vesa_mode_info->physbase & (MM_PAGESZ - 1));
            con_data = (uint16_t *) (VESA_FB_VIRT + vesa_fb_pages * MM_PAGESZ);

            con_width = vesa_mode_info->x_res / vesa_char_width;
            con_height = vesa_mode_info->y_res / vesa_char_height;

            // Clear it with red color
            memsetl((void *) vesa_fb_base, 0, vesa_fb_size / 4);
        }
    }

    if (vesa_fb_base) {
        const uint8_t *data = (const uint8_t *) logo_header_data;
        uint8_t pixel[4];
        for (int y = 0; y < logo_height; ++y) {
            for (int x = 0; x < logo_width; ++x) {
                LOGO_HEADER_PIXEL(data, pixel);

                for (int i = 0; i < 2; ++i) {
                    for (int j = 0; j < 2; ++j) {
                        x86_vesa_set(x * 2 + i, y * 2 + j, (pixel[0] << 16) | (pixel[1] << 8) | pixel[2]);
                    }
                }
            }
        }

        con_cury += (logo_height * 2) / vesa_char_height;
    }
#endif

    memsetw(con_data, 0x0700, con_width * con_height);
}
