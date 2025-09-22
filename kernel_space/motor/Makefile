# Makefile for Motor Kernel Module & Device Tree Overlay
# 編譯後 .ko 統一放 modules, .dtbo 複製到 /boot/firmware/overlays/

# 需要編譯的 module
obj-m += motorv1.o

# include 路徑
EXTRA_CFLAGS += -I$(PWD)/../kernel_include

# kernel build 目標
KDIR := /lib/modules/$(shell uname -r)/build

# 目標路徑
PWD := $(shell pwd)

# 統一放置 .ko 檔的目錄
KO_DIR := $(PWD)/../../modules

# Device Tree Overlay 檔案
DT_SRC := motor.dts dtree.dts
DT_DEST := /boot/firmware/overlays


# 編譯 module 與 dtbo 
all:
	@echo "****** Building Kernel Module ******"
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	mkdir -p $(KO_DIR)
	cp -v *.ko $(KO_DIR)/
	@echo "****** Kernel Module copied to $(KO_DIR) ******"

	@echo "****** Building Device Tree Overlay ******"
	for dts in $(DT_SRC); do \
		dtbo=$${dts%.dts}.dtbo; \
		dtc -@ -I dts -O dtb -o $$dtbo $$dts; \
		sudo cp -v $$dtbo $(DT_DEST)/; \
	done	
	@echo "****** Device Tree Overlay copied to $(DT_DEST) ******"

	@echo "****** Cleaning intermediate files ******"
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f *.dtbo
	@echo "Build and copy finished!"

# 清理
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f *.dtbo
