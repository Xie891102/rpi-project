#CROSS_COMPILE=aarch64-linux-gnu-
CC=$(CROSS_COMPILE)gcc
obj-m += buzzer_k.o
KDIR=/lib/modules/$(shell uname -r)/build
#EXTRA_CFLAGS += -DGPIO23

# 統一 .ko 輸出資料夾
PWD := $(shell pwd)
KO_DIR := $(PWD)/../../modules

EXTRA_CFLAGS += -I/home/pi/rpi_project/kernel_space/kernel_include


all:
	@echo "****** Building buzzer Module ******"
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	mkdir -p $(KO_DIR)
	cp -v *.ko $(KO_DIR)/
	@echo "****** Created KO_DIR and Copy .ko file successed  ******"
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	@echo "Build successed and has cleaned other files~"

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean