#!/bin/bash

# 總腳本(卸載模組): 依序卸載 buzzer、tcrt5000(紅外線)、hc-sr04(超聲波)、馬達控制器

# 設定模組腳本所在的資料夾
SCRIPT_DIR="/home/pi/rpi_project/kernel_space/kernel_script"

echo ">>> 開始卸載所有模組..."


# 1. 卸載蜂鳴器(Buzzer)模組
if ! $SCRIPT_DIR/buzzy_unload.sh; then
    echo "[ERROR] buzzy_unload.sh 卸載失敗"
    exit 1   # 如果失敗結束腳本
fi


# 2. 卸載循跡測器(tcrt5000)模組
if ! $SCRIPT_DIR/tcrt_unload.sh; then
    echo "[ERROR] tcrt_unload.sh 卸載失敗"
    exit 1
fi


# 3. 卸載超聲波 (HC-SR04)
if ! $SCRIPT_DIR/hc_sr04_unload.sh; then
    echo "[ERROR] hc_sr04_unload.sh 卸載失敗"
    exit 1
fi

# 4. 卸載馬達控制器(l298n)
if ! $SCRIPT_DIR/motor_unload.sh; then
    echo "[ERROR] motor_unload.sh 卸載失敗"
    exit 1
fi


# 5. 全部模組卸載成功
echo ">>> 所有模組卸載成功!"
exit 0
