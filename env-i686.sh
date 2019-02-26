#!/bin/bash

export TARGET=i686-elf
export T=/storage/toolchains/${TARGET}
export PATH="${T}/bin:${PATH}"
export CROSS_COMPILE=${TARGET}-
export ARCH=x86
unset BOARD

