#!/bin/bash

# �`�}���\��(�����Ҳ�): �̧Ǹ��J buzzer�Btcrt5000�BHC-SR04 (�W�n�i) 

# �]�w�Ҳո}���Ҧb����Ƨ� (�קK�]�����|���P�䤣���ɮ�)
SCRIPT_DIR="/home/pi/rpi_project/kernel_space/kernel_script"

echo ">>> �}�l���J�Ҧ��Ҳ�..."


# 1. ���J���ﾹ (Buzzer) �Ҳ�
if ! $SCRIPT_DIR/buzzy_load.sh; then
    echo "!!! buzzy_load.sh ���J����"
    exit 1   # �p�G���ѴN�����}�� (�קK�᭱�Ҳզ��̿�o�X��)
fi


# 2. ���J�`��P���� (TCRT5000) �Ҳ�
if ! $SCRIPT_DIR/tcrt_load.sh; then
    echo "!!! tcrt_load.sh ���J����"
    exit 1
fi


# 3. ���J�W�n�i�Ҳ� (HC-SR04)
if ! $SCRIPT_DIR/hc_sr04_load.sh; then
    echo "!!! hc_sr04_load.sh ���J����"
    exit 1
fi


# 4. �����Ҳճ����J���\
echo ">>> �Ҧ��Ҳդw���\���J�I"
exit 0
