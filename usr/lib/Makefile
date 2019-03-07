LIBS=libc \
	 libcrt

all: makelibs

makelibs:
	$(foreach lib,$(LIBS),make -s -C $(lib) CC=$(CC) AS=$(AS) AR=$(AR) O=$(O) all;)
