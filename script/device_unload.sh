#!/bin/bash

# 關機自動執行的腳本 
# 卸載: 紅外線循跡 + 馬達控制器 + 超聲波測距


# 卸載 TCRT5000 Device Driver 模組
echo ">>> Unloading TCRT5000 Device Driver..."
if lsmod | grep -q tcrt5000_driver; then
	rmmod tcrt5000_driver
	echo "TCRT5000 Device Driver Unload Successful"
else 
	echo "Warning! TCRT5000 Device Driver was not unloaded"
fi




# 卸載 TCRT5000 HAL 模組 
echo ">>> Unloading TCRT5000 HAL..."
if lsmod | grep -q tcrt5000_hal; then
	rmmod tcrt5000_hal
	echo "TCRT5000 HAL Unload Successful"
else
	echo "Warning! TCRT5000 HAL was not unloaded"
fi

echo ">>> Unload script finished"

