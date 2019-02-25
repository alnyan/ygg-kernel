
ifneq ($(ENABLE_RTL8139),)
DEFINES+=-DENABLE_RTL8139
OBJS+=build/dev/pci/net/rtl8139.o
DIRS+=build/dev/pci/net
endif
