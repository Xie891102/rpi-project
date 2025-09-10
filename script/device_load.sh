#!/bin/bash

# 開機自動執行的腳本 
# 掛載: 紅外線循跡 + 馬達控制器 + 超聲波測距


# 1.定義模組路徑
MODULE_DIR="/home/pi/rpi_project/modules"
HAL_MODULE="$MODULE_DIR/tcrt5000_hal.ko"
DRIVER_MODULE="$MODULE_DIR/tcrt5000_driver.ko"


# 2. 載入 HAL 模組
echo ">>> Loading TCRT5000 HAL..."
if ! (lsmod | grep -q tcrt5000_hal); then
	insmod "$HAL_MODULE"			# 載入 hal.ko 模組
	echo "HAL loaded"
else 
	echo "HAL already loaded"		# 若已載入 顯示訊息
fi



# 3. 載入 Device Driver 模組
echo ">>> Loading TCRT5000 Device Driver..."
if ! (lsmod | grep -q tcrt5000_driver); then
	insmod "$DRIVER_MODULE"			# 載入 driver.ko 模組
	echo "Device Driver loaded"
else
	echo "Device Driver already loaded"	# 若已載入 顯示訊息
fi



# 4. 檢查所有模組是否有失敗的
if ! (lsmod | grep -q tcrt5000_hal); then 
	echo "TCRT5000 HAL load failed!Please check out"
elif ! (lsmod | grep -q tcrt5000_driver); then
	echo "Tcrt5000 Device Driver load failed!Please check out"
else
	echo ">>> All modules load successed"
fi




