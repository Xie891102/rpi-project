# Makefile for TCRT5000 User-Space Program
# 編譯出執行檔 tcrt5000 並放到 car_elf 目錄
# 不刪除中間物件檔（.o）

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
# 預設目標：編譯執行檔
all: $(TARGET)
	@echo "****** Compiling user-space program ******"
	mkdir -p $(TARGET_DIR)
	cp -v $(TARGET) $(TARGET_DIR)/
	@echo "****** Executable copied to $(TARGET_DIR)/$(TARGET) ******"

# 編譯規則：連結所有 .o 成最終執行檔
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

# .c -> .o 編譯規則
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# clean 只刪除最終執行檔，不刪除 .o
clean:
	rm -f $(TARGET)

# 清理中間檔（如果真的需要手動刪）
clean_objs:
	rm -f $(OBJS)

.PHONY: all clean clean_objs
