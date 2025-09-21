#!/bin/bash

# 羆竲セ(更家舱): ㄌ更 buzzertcrt5000HC-SR04 (禬羘猧) 

# 砞﹚家舱竲セ┮戈Ж
SCRIPT_DIR="/home/pi/rpi_project/kernel_space/kernel_script"

echo ">>> 秨﹍更┮Τ家舱..."


# 1. 更噶伙竟 (Buzzer) 家舱
if ! $SCRIPT_DIR/buzzy_unload.sh; then
    echo "!!! buzzy_unload.sh 更ア毖"
    exit 1   # 狦ア毖碞挡竲セ
fi


# 2. 更碻格稰代竟 (TCRT5000) 家舱
if ! $SCRIPT_DIR/tcrt_unload.sh; then
    echo "!!! tcrt_unload.sh 更ア毖"
    exit 1
fi


# 3. 更禬羘猧家舱 (HC-SR04)
if ! $SCRIPT_DIR/hc_sr04_unload.sh; then
    echo "!!! hc_sr04_unload.sh 更ア毖"
    exit 1
fi


# 4. 场家舱常更Θ
echo ">>> ┮Τ家舱Θ更"
exit 0
