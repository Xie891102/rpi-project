#!/bin/bash

# 羆竲セ(本更家舱): ㄌ更 buzzertcrt5000HC-SR04 (禬羘猧) 

# 砞﹚家舱竲セ┮戈Ж (磷隔畖ぃтぃ郎)
SCRIPT_DIR="/home/pi/rpi_project/kernel_space/kernel_script"

echo ">>> 秨﹍更┮Τ家舱..."


# 1. 更噶伙竟 (Buzzer) 家舱
if ! $SCRIPT_DIR/buzzy_load.sh; then
    echo "!!! buzzy_load.sh 更ア毖"
    exit 1   # 狦ア毖碞挡竲セ (磷家舱Τㄌ苦玱岿)
fi


# 2. 更碻格稰代竟 (TCRT5000) 家舱
if ! $SCRIPT_DIR/tcrt_load.sh; then
    echo "!!! tcrt_load.sh 更ア毖"
    exit 1
fi


# 3. 更禬羘猧家舱 (HC-SR04)
if ! $SCRIPT_DIR/hc_sr04_load.sh; then
    echo "!!! hc_sr04_load.sh 更ア毖"
    exit 1
fi


# 4. 场家舱常更Θ
echo ">>> ┮Τ家舱Θ更"
exit 0
