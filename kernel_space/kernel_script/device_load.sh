#!/bin/bash

# 總腳本(掛載功能): 依序載入buzzer、tcrt5000(紅外線)、HC-SR04(超聲波)、馬達控制器 

# 設定模組腳本所在的資料夾(避免路徑不同找不到檔案)
SCRIPT_DIR="/home/pi/rpi_project/kernel_space/kernel_script"

echo ">>> 開始載入所有模組..."


# 1. 載入蜂鳴器(Buzzer)模組
if ! $SCRIPT_DIR/buzzy_load.sh; then
    echo "[ERROR] buzzy_load.sh 載入失敗"
    exit 1   # 如果失敗結束腳本
fi


# 2. 載入循跡感測器(TCRT5000)模組
if ! $SCRIPT_DIR/tcrt_load.sh; then
    echo "[ERROR] tcrt_load.sh 載入失敗"
    exit 1
fi


# 3. 載入超聲波(HC-SR04)模組
if ! $SCRIPT_DIR/hc_sr04_load.sh; then
    echo "[ERROR] hc_sr04_load.sh 載入失敗"
    exit 1
fi


# 4. 載入馬達控制器(l298n)模組
if ! $SCRIPT_DIR/motor_load.sh; then
    echo "[ERROR] motor_load.sh 載入失敗"
    exit 1
fi

# 5. 全部模組載成功
echo ">>> 所有模組載入成功"
exit 0
