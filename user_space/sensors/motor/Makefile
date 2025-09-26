# 編譯器與參數
CC = gcc
CFLAGS = -Wall -I$(HOME)/rpi_project/user_space/userspace_includes

# 目標
TARGET = test_motor

# 原始碼
SRCS = test_motor.c
OBJS = $(SRCS:.c=.o)

# 預設規則：編譯執行檔後印出 success
all: $(TARGET)
	@echo "*********** make success ***********"

# 編譯並產生執行檔 (編完後刪掉 .o)
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -c $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)
	rm -f $(OBJS)

# 清除：刪掉執行檔與所有中間檔
clean:
	rm -f $(TARGET) $(OBJS)
