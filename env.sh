#!/bin/bash

export TARGET=aarch64-linux-gnu
export T=/storage/toolchains/${TARGET}
export PATH="${T}/bin:${PATH}"
export CROSS_COMPILE=${TARGET}-
export ARCH=aarch64
export BOARD=bcm2837
export QEMU_BIN=/storage/toolchains/aarch64-debug-qemu/bin/qemu-system-aarch64
