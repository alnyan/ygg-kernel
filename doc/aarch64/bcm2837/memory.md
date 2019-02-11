BCM283x (BCM2835/6/7) Memory Map
================================

Physical memory is divided into the following blocks:

    - 0x00000000 - `VC_ARM_BOUNDARY` - SDRAM
    - `VC_ARM_BOUNDARY` - `VC_SDRAM_END` - VC SDRAM
    - `VC_SDRAM_END` - `IO_BASE` - RAM
    - `IO_BASE` - `IO_BASE + 0x1000000` - IO Peripherals
    - `IO_BASE + 0x1000000` - RAM

Which are then mapped by MMU and kernel:

    - 0x00000000 - 0xC0000000 - Userspace-available
    - 0xC0000000 - 0xEFFFFFFF - Kernel memory
    - 0xF0000000 - 0xFFFFFFFF
