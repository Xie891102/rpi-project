# Makefile - 編譯 test_straight 並放到 car_elf，不產生 .o

CC = gcc
CFLAGS = -Wall -I/home/pi/rpi_project/user_space/userspace_includes

# 源檔
SRC = test_straight.c /home/pi/rpi_project/user_space/sensors/motor/motor.c

# 執行檔
TARGET_DIR = /home/pi/rpi_project/car_elf
TARGET = $(TARGET_DIR)/test_straight

# -----------------------
# 默認目標
# -----------------------
all: $(TARGET)

$(TARGET): $(SRC)
	@echo "[INFO] 編譯中..."
	# 如果目標目錄不存在就建立
	@mkdir -p $(TARGET_DIR)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)
	@echo "************** make success - test_straight **************"

# 清除目標
clean:
	@rm -f *.o     # 刪掉同目錄下所有物件檔
	@rm -f *~      # 刪掉編輯器備份檔	
	@echo "[INFO] 清除完成"

.PHONY: all clean
