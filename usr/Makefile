LIBUSR_OBJS=$(O)/lib/printf.o \
			$(O)/lib/unistd.o \
			$(O)/lib/string.o
USR_PROGS=$(O)/bin/init \
		  $(O)/bin/hello

DIRS=$(O)/lib \
	 $(O)/bin \
	 $(O)/etc

all: mkdirs $(O)/lib/libusr.a $(O)/init.tar

mkdirs:
	mkdir -p $(DIRS)

$(O)/init.tar: $(USR_PROGS)
	rm -rf $(O)/etc
	cp -r etc $(O)/etc
	tar -C $(O) -cf $@ bin etc

$(O)/bin/%: bin/%.c
	$(CC) -Ilib -nostdlib -ffreestanding -ggdb -o $@ $< $(O)/lib/libusr.a -lgcc

$(O)/lib/libusr.a: $(LIBUSR_OBJS)
	$(AR) rcs $@ $(LIBUSR_OBJS)

$(O)/lib/%.o: lib/%.c
	$(CC) -nostdlib -c -ffreestanding -o $@ $<

