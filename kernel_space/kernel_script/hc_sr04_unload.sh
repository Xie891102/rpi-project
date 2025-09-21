#!/bin/sh

module="hc_sr04"
device="ultrasonic"

# 卸載驅動
/sbin/rmmod $module || exit 1

# 移除 /dev/ultrasonic* 裝置節點
rm -f /dev/${device}*