#CROSS_COMPILE=aarch64-linux-gnu-
CC=$(CROSS_COMPILE)gcc
obj-m += motorv1.o
KDIR=/lib/modules/$(shell uname -r)/build
PWD=$(shell pwd)
EXTRA_CFLAGS += -D__READL_WRITEL__
#EXTRA_CFLAGS += -DGPIO21

all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean

