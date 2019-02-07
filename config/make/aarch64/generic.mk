
LINKER=config/ld/aarch64/generic.ld

OBJS+=build/arch/aarch64/boot.o
DIRS+=build/arch/aarch64

CFLAGS+=-DARCH_AARCH64

all:

info-arch:
	@echo "Target architecture is aarch64"
