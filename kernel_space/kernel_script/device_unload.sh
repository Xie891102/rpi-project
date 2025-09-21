#!/bin/bash

# �`�}���\��(�����Ҳ�): �̧Ǩ��� buzzer�Btcrt5000�BHC-SR04 (�W�n�i) 

# �]�w�Ҳո}���Ҧb����Ƨ�
SCRIPT_DIR="/home/pi/rpi_project/kernel_space/kernel_script"

echo ">>> �}�l�����Ҧ��Ҳ�..."


# 1. �������ﾹ (Buzzer) �Ҳ�
if ! $SCRIPT_DIR/buzzy_unload.sh; then
    echo "!!! buzzy_unload.sh ��������"
    exit 1   # �p�G���ѴN�����}��
fi


# 2. �����`��P���� (TCRT5000) �Ҳ�
if ! $SCRIPT_DIR/tcrt_unload.sh; then
    echo "!!! tcrt_unload.sh ��������"
    exit 1
fi


# 3. �����W�n�i�Ҳ� (HC-SR04)
if ! $SCRIPT_DIR/hc_sr04_unload.sh; then
    echo "!!! hc_sr04_unload.sh ��������"
    exit 1
fi


# 4. �����Ҳճ��������\
echo ">>> �Ҧ��Ҳդw���\�����I"
exit 0
