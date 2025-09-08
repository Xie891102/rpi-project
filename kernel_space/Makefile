# Makefile for TCRT5000 HAL & Device Driver
# 對應 kernel module 編譯後，會產生 .ko 檔

# 需要編譯的 module **************************
obj-m += tcrt5000_hal.o
obj-m += tcrt5000_driver.o


# 告訴編譯器 include/gpio_pins.h 的路徑
EXTRA_CFLAGS += -I$(PWD)/../include


# 指定 kernel buld 目錄 ***********************

# 指向當前 kernel 的 build 資料夾(內有標頭檔/Makefile)，這樣才能編譯 module
KDIR := /lib/modules/$(shell uname -r)/build	

# 取得當前路徑，讓kernel build system 找到 .c 檔案
PWD := $(shell pwd)

# 編譯 ****************************************(會呼叫 kernel build 系統編譯 module)
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# 清理 ****************************************(清除 .o .ko .mod.c)
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean




