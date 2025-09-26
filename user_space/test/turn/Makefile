# ------------------------------
# Makefile for turn
# ------------------------------

# 執行檔名稱
TARGET = test_turn

# 源碼
SRC = test_turn.c
MOTOR_SRC = ../../sensors/motor/motor.c

# 標頭檔目錄
INCLUDE = -I../../userspace_includes

# 目標執行檔位置
BIN_DIR = /home/pi/rpi_project/car_elf
BIN = $(BIN_DIR)/$(TARGET)

# 編譯器與參數
CC = gcc
CFLAGS = -Wall $(INCLUDE)

# ------------------------------
# 編譯規則
# ------------------------------

all: $(BIN)

$(BIN): $(SRC) $(MOTOR_SRC)
	@echo "[INFO] 編譯中..."
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(SRC) $(MOTOR_SRC) -o $(BIN)
	@echo "************** make success - $(TARGET) **************"

# ------------------------------
# 清除中間產物
# ------------------------------
clean:
	@rm -f *.o
	@echo "[INFO] 清除完成"
