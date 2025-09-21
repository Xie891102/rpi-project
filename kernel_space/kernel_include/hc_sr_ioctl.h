#ifndef SONIC_CHARDEV_H
#define SONIC_CHARDEV_H

#include <linux/ioctl.h>

// ioctl 的魔術數字及命令編號
#define HC_SR04_IOC_MAGIC 'H'
#define HC_SR04_SET_TRIGGER _IOW(HC_SR04_IOC_MAGIC, 1, int) // 設定 trigger 腳位
#define HC_SR04_SET_ECHO    _IOW(HC_SR04_IOC_MAGIC, 2, int) // 設定 echo 腳位

#endif
