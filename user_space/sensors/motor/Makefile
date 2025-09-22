# Makefile for L298N Motor User-space Controller

# 編譯器與標準
CC := gcc
INCLUDE_DIR := /home/pi/rpi_project/user_space/userspace_includes

CFLAGS := -I$(INCLUDE_DIR) -Wall

# 可執行檔名稱
TARGET := motor_app

# 原始碼檔案
SRCS := motor.c

.PHONY: all clean

# 預設目標
all: $(TARGET)

# 編譯可執行檔
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS)

# 清理
clean:
	rm -f $(TARGET)
