ENTRY(_start)

SECTIONS {
    . = 0x400000;

    .text : ALIGN(4K)
    {
        *(.text)
    }

    /* TODO: add stuff like .data and .bss */
}
