#!/bin/sh
set -x  # 顯示每個指令（方便除錯）

B26=`uname -r | cut -d'.' -f2`

if [ "$B26" -eq "4" ]; then
  module="/home/pi/rpi_project/modules/hc_sr04.o"
else
  module="/home/pi/rpi_project/modules/hc_sr04.ko"
fi

device="ultrasonic"   # 裝置名稱前綴
group="root"
mode="664"

# 載入驅動模組
/sbin/insmod $module || exit 1

# 取得 major number
major=`cat /proc/devices | awk "\\$2==\"$device\" {print \\$1}"`
echo "Major number = $major"

# 刪除舊的 /dev/ultrasonic* 節點
rm -f /dev/${device}*

# 建立 4 個 /dev/ultrasonic0 ~ /dev/ultrasonic3
for i in 0 1 2 3
do
  mknod "/dev/${device}$i" c $major $i
done

chgrp $group /dev/${device}*
chmod $mode  /dev/${device}*
