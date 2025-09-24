# Makefile for TCRT5000 User-Space Program
# 會編譯出執行檔 tcrt5000 並放到 car_elf 目錄

# 編譯器 & 選項
CC := gcc
CFLAGS := -Wall -g
INCLUDES := -I$(PWD)/../../userspace_includes

# 來源檔案
SRCS := test_tcrt5000.c tcrt5000.c tcrt5000_thread.c

# 物件檔（中間檔）
OBJS := $(SRCS:.c=.o)

# 最終執行檔
TARGET := tcrt5000
TARGET_DIR := $(PWD)/../../car_elf

# -----------------------------
# 編譯 & 移動執行檔
all: $(TARGET)
	@echo "****** Compiling user-space program ******"
	mkdir -p $(TARGET_DIR)
	cp -v $(TARGET) $(TARGET_DIR)/
	@echo "****** Executable copied to $(TARGET_DIR)/$(TARGET) ******"
	$(MAKE) clean_objs

# 編譯規則
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 清理所有中間檔 & 執行檔
clean:
	rm -f $(OBJS) $(TARGET)

# 清理中間檔
clean_objs:
	rm -f $(OBJS)

.PHONY: all clean clean_objs
