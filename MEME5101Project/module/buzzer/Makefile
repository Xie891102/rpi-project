#CROSS_COMPILE=aarch64-linux-gnu-
CC=$(CROSS_COMPILE)gcc
obj-m += buzzer_k.o
KDIR=/lib/modules/$(shell uname -r)/build
#EXTRA_CFLAGS += -DGPIO23

all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean


