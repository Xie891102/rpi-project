# Makefile for test_logic.c
# 會編譯 test_logic.c，連結 motor 與 tcrt5000 的 .o 檔，輸出到 car_elf

# 編譯器 & 選項
CC := gcc
CFLAGS := -Wall -Iuserspace_includes -g

# 來源檔案
SRCS := test_logic.c

# 模組檔案（已經編譯好的 .o）
OBJS := \
    sensors/motor/motor.o \
    sensors/tcrt5000/tcrt5000.o \
    sensors/tcrt5000/tcrt5000_thread.o

# 執行檔
TARGET := car_elf/test_car

.PHONY: all clean

all: $(TARGET)

# 編譯規則
$(TARGET): $(SRCS) $(OBJS)
	@mkdir -p car_elf
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(OBJS) -lpthread
	@echo "****** Executable created: $(TARGET) ******"

# 清理僅執行檔，保留中間檔
clean:
	rm -f $(TARGET)
