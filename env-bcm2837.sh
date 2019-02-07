#!/bin/bash

export TARGET=aarch64-linux-gnu
export T=/storage/toolchains/${TARGET}
export PATH="${T}/bin:${PATH}"
export CROSS_COMPILE=${TARGET}-
