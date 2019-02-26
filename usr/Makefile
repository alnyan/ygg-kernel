LIBUSR_OBJS=$(O)/lib/printf.o \
			$(O)/lib/unistd.o \
			$(O)/lib/string.o
USR_PROGS=$(O)/bin/init \
		  $(O)/bin/hello
CONFIGS=$(O)/etc/network.conf

DIRS=$(O)/lib \
	 $(O)/bin \
	 $(O)/etc

all: mkdirs $(O)/lib/libusr.a $(O)/init.tar

mkdirs:
	mkdir -p $(DIRS)

$(O)/init.tar: $(USR_PROGS) $(CONFIGS)
	@printf " ROOTFS\t%s\n" "init.tar"
	@tar -C $(O) -cf $@ bin etc

$(O)/etc/%: etc/%
	@printf " CONF\t%s\n" "/$<"
	@cp "$<" "$@"

$(O)/bin/%: bin/%.c
	@printf " CC\t%s\n" "/$(<:%.c=%)"
	@$(CC) -Ilib -nostdlib -ffreestanding -ggdb -o $@ $< $(O)/lib/libusr.a -lgcc

$(O)/lib/libusr.a: $(LIBUSR_OBJS)
	@printf " AR\t%s\n" "usr/lib/libusr.a"
	@$(AR) rcs $@ $(LIBUSR_OBJS)

$(O)/lib/%.o: lib/%.c
	@printf " CC\t%s\n" "usr/$(<:%.c=%.o)"
	@$(CC) -nostdlib -c -ffreestanding -o $@ $<

