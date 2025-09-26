#ifndef BUZZER_CHARDEV_H
#define BUZZER_CHARDEV_H

#include <linux/ioctl.h>

#define MAJOR_NUM 0
#define IOCTL_SET_BUZZER_ON _IO(230,10)
#define IOCTL_SET_BUZZER_OFF _IO(230,11)
#define IOCTL_TOGGLE_BUZZER _IOW(230,12,int)
#define DEVICE_FILE_NAME "/dev/buzzer0"

#endif
