# 統一Makefile 
# main.c、logic.c、/sensors/.c檔

# 編譯器選擇
CC = gcc
CFLAGS= -Wall -O2 -pthread

# 專案路徑及執行檔檔名
SRC = main.c logic.c sensors/tcrt5000.c
OBJ = $(SRC:.c=.o)
TARGET = /home/pi/rpi_project/car_elf/car_app 


# 預設目標
all: $(TARGET)

# 連結產生最終檔
$(TARGET): $(OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $(OBJ)
	@echo "編譯後的執行檔在 $(TARGET)"
	rm -f $(OBJ)


# 編譯 .c -> .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ 


# 清理中間檔
clean: 
	rm -f *.o sensors/*.o logic.o main.o
	@echo "中間檔已清除完"

