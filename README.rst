***********
Valhalla OS
***********

0. Overview
###########

This is a repository of Valhalla OS, which consists of Yggdrasil kernel, varoius utilities, as well
as a bunch of userspace programs intended to run in it.

1. Features
###########

General features:
   * The core kernel code tries to be as platform-independent as possible, so the same code will run
     on both aarch64 boards and x86 computers, allowing to write platform-specific code without
     breaking other platforms
   * Basic networking support - ARP route resolution for network interfaces
   * Virtual filesystem, devfs (featuring nodes for ttyN, ttySN, ram0 etc.)
   * Initramfs implemented using tar
   * ELF execution (execve(...) syscall)
   * fork()/getpid()/other syscalls are supported too
   * Basic filesystem support for userspace via syscalls:
     * open()/read()/write()/close()
     * opendir()/readdir()/closedir()
   * Nanosecond-resolution system time and nanosleep() system call (if possible on target platform)
   * Logging levels for debug output

x86-specific features:
   * Support for PCI devices
   * RTL8139 PCI support (both Tx/Rx)
   * HPET support (if available on platform, PIT will be used as fallback otherwise)
   * RTC
   * Both 4KiB and 4MiB page support, although it's recommended to use 4KiB pages for userspace
     mappings
   * VESA framebuffer support with graphics and text output
   * Serial port debugging

2. Plans
########

General:
   * Expand userspace functionality with more syscalls
   * Better functionality for virtual filesystems like procfs/devfs
   * Threads and synchronization primitives
   * PCI IDE controller support

x86:
   * x86-64 support via additional loader stage
   * syscall memory space optimizations

aarch64/bcm2837:
   * MMU support
