# ========================================
# Makefile for straight_tcrt1 (馬達 + TCRT5000)
# ========================================

# 編譯器與旗標
CC      := gcc
CFLAGS  := -Wall -I/home/pi/rpi_project/user_space/userspace_includes
LDFLAGS := -lpthread   # pthread 用於 tcrt5000_thread.c

# 來源檔案
# - test_straight_tcrt1.c: 主程式
# - motor.c: 馬達控制
# - tcrt5000.c / tcrt5000_thread.c: TCRT5000 循跡感測
SRC := test_straight_tcrt1.c \
       ../../sensors/motor/motor.c \
       ../../sensors/tcrt5000/tcrt5000.c \
       ../../sensors/tcrt5000/tcrt5000_thread.c

# 目標執行檔路徑
TARGET := /home/pi/rpi_project/car_elf/test_straight_tcrt1

# ========================================
# 預設目標
all: $(TARGET)
	@echo "************** make success - test_straight_tcrt1 **************"

# ========================================
# 編譯連結目標
$(TARGET): $(SRC)
	@echo "[INFO] 編譯中..."
	# 確保目標資料夾存在
	@mkdir -p $(dir $(TARGET))
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

# ========================================
# 清除中間檔案
# 只刪除 .o，不會刪 .c 或 Makefile
clean:
	@rm -f *.o
	@echo "[INFO] 中間檔案清除完成"

# ========================================
# 宣告不對應實際檔案的目標
.PHONY: all clean
