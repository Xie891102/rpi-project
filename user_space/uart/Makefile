# 編譯器與旗標
CC = gcc
CFLAGS = -Wall -O2 -pthread -I../userspace_includes

# 專案檔案
SRC = test.c uart.c uart_thread.c uart_queue.c
OBJ = $(patsubst %.c,%.o,$(SRC))
TARGET = /home/pi/rpi_project/car_elf/uart   # <-- 執行檔名稱改成 uart

# 預設目標
all: $(TARGET)

# 連結產生最終執行檔
$(TARGET): $(OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $(OBJ)
	@echo "編譯完成，執行檔在 $(TARGET)"
	rm -f $(OBJ)    # 刪掉中間檔 .o，不碰 .c

# 編譯 .c -> .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 一鍵編譯並執行
run: $(TARGET)
	sudo $(TARGET)

# 清理
clean:
	rm -f $(OBJ) $(TARGET)
	@echo "中間檔與執行檔已清除完"
