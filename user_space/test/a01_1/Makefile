# 改良版: 直行 更細緻回復調整(含線性的歷史紀錄)

# Makefile for a01_1_test
# ------------------------
# 來源檔案目錄結構：
# - main: a01_1_test.c (本目錄)
# - motor: ../../sensors/motor/motor.c
# - tcrt5000: ../../sensors/tcrt5000/tcrt5000.c + ../../sensors/tcrt5000/tcrt5000_thread.c
# - 標頭檔: ../../userspace_includes

# 編譯器與參數
CC = gcc
CFLAGS = -Wall -I../../userspace_includes
LDFLAGS = -lpthread

# 執行檔放置目錄
TARGET_DIR = /home/pi/rpi_project/car_elf
TARGET = $(TARGET_DIR)/a01_1_test

# 來源檔
SRCS = a01_1_test.c \
       ../../sensors/motor/motor.c \
       ../../sensors/tcrt5000/tcrt5000.c \
       ../../sensors/tcrt5000/tcrt5000_thread.c

# ========================
# 編譯目標
# ========================
all: $(TARGET)

$(TARGET): $(SRCS)
	@echo "[INFO] 編譯中..."
	@mkdir -p $(TARGET_DIR)                  # 如果目標目錄不存在就建立
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)
	@echo "************** make success - a01_1_test(改良版1) **************"

# ========================
# 清理目標
# ========================
clean:
	@rm -f $(TARGET)
	@echo "[INFO] 清除完成"
