#pragma once

void x86_con_putc(char c);
void x86_con_init(void);

#if defined(ENABLE_VESA_FBCON)
void x86_vesa_con_init(void);
#endif
