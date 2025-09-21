#CROSS_COMPILE=aarch64-linux-gnu-
CC=$(CROSS_COMPILE)gcc
obj-m += hc_sr04.o
KDIR=/lib/modules/$(shell uname -r)/build
#EXTRA_CFLAGS += -DGPIO23

PWD := $(shell pwd)

# 統一 .ko 輸出資料夾
KO_DIR := $(PWD)/../../modules

# include 路徑（如果有共用 header）
EXTRA_CFLAGS += -I/home/pi/rpi_project/kernel_space/kernel_include

all:
	@echo "****** Building HC-SR04 Module ******"
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	mkdir -p $(KO_DIR)
	cp -v *.ko $(KO_DIR)/
	@echo "****** Created KO_DIR and Copy .ko file successed  ******"
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	@echo "Build successed and has cleaned other files~"

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean