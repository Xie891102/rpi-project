# Makefile for TCRT5000 HAL & Device Driver
# 對應 kernel module 編譯後，會產生 .ko 檔

# 需要編譯的 module **************************
obj-m += tcrt5000_hal.o
obj-m += tcrt5000_driver.o


# 告訴編譯器 include/gpio_pins.h 的路徑
EXTRA_CFLAGS += -I$(PWD)/../kernel_include


# 指定 kernel buld 目錄 ***********************

# 指向當前 kernel 的 build 資料夾(內有標頭檔/Makefile)，這樣才能編譯 module
KDIR := /lib/modules/$(shell uname -r)/build	

# 取得當前路徑，讓kernel build system 找到 .c 檔案
PWD := $(shell pwd)

# 統一放置 .ko 檔的目錄
KO_DIR := $(PWD)/../../modules


# 編譯 ****************************************(會呼叫 kernel build 系統編譯 module)
# 將 .ko 檔移至指定目錄下，並將中間產物清除掉
all:
	@echo "****** Building Modules ******"
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	mkdir -p $(KO_DIR)
	cp -v *.ko $(KO_DIR)/
	@echo "****** Created KO_DIR and Copy .ko file successed  ******"
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	@echo "Build successed and has cleaned other files~"


# 清理 ****************************************(清除 .o .ko .mod.c)
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean




