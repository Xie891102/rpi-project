#ifndef __GPIO_ADDRESS__
#define __GPIO_ADDRESS__

#define GPIO_BASE 0xFE200000

#define GPFSEL0 0x00
#define GPFSEL1 0x04
#define GPFSEL2 0x08
#define GPFSEL3 0x0c
#define GPFSEL4 0x10
#define GPFSEL5 0x14
#define GPSET0 0x1c
#define GPSET1 0x20
#define GPCLR0 0x28
#define GPCLR1 0x2c
#define GPLEV0 0x34
#define GPLEV1 0x38
#define GPEDS0 0x40
#define GPEDS1 0x44
#define GPREN0 0x4c
#define GPREN1 0x50
#define GPFEN0 0x58
#define GPFEN1 0x5C
#define GPHEN0 0x64
#define GPHEN1 0x68
#define GPLEN0 0x70
#define GPLEN1 0x74
#define GPAREN0 0x7c
#define GPAREN1 0x80
#define GPAFEN0 0x88
#define GPAFEN1 0x8c
#define GPIO_PUP_PDN_CNTRL_REG0 0xe4
#define GPIO_PUP_PDN_CNTRL_REG1 0xe8
#define GPIO_PUP_PDN_CNTRL_REG2 0xec
#define GPIO_PUP_PDN_CNTRL_REG3 0xf0

#endif // #define def __GPIO_ADDRESS__

#ifndef _MOTOR_GPIO_H_
#define _MOTOR_GPIO_H_

/* BCM2711 (Pi4) GPIO 暫存器定義 */
#define BCM2711_GPIO_BASE  0xFE200000
#define GPIO_SIZE          0x1000

/* L298N GPIO 腳位定義 */
#define MOTOR_IN1_GPIO    17    // 左馬達方向控制1
#define MOTOR_IN2_GPIO    27    // 左馬達方向控制2  
#define MOTOR_IN3_GPIO    22    // 右馬達方向控制1
#define MOTOR_IN4_GPIO    23    // 右馬達方向控制2
#define PWM_LEFT_GPIO     18    // 左馬達PWM控制 (GPIO18 = PWM0 channel 0)
#define PWM_RIGHT_GPIO    19    // 右馬達PWM控制 (GPIO19 = PWM0 channel 1)

/* GPIO 暫存器偏移 */
#define GPFSEL0   0x00    // GPIO功能選擇暫存器0 (GPIO 0-9)
#define GPFSEL1   0x04    // GPIO功能選擇暫存器1 (GPIO 10-19) 
#define GPFSEL2   0x08    // GPIO功能選擇暫存器2 (GPIO 20-29)
#define GPSET0    0x1C    // GPIO設定暫存器0 (設為高電位)
#define GPCLR0    0x28    // GPIO清除暫存器0 (設為低電位)

/* ioctl 指令定義 */
#define MOTOR_IOC_MAGIC  'M'
#define IOCTL_SET_SPEED_LEFT    _IOW(MOTOR_IOC_MAGIC, 0, int)      // 設定左馬達速度 (0-100%)
#define IOCTL_SET_SPEED_RIGHT   _IOW(MOTOR_IOC_MAGIC, 1, int)      // 設定右馬達速度 (0-100%)
#define IOCTL_LEFT_FORWARD      _IO(MOTOR_IOC_MAGIC, 2)            // 左馬達前進
#define IOCTL_LEFT_BACKWARD     _IO(MOTOR_IOC_MAGIC, 3)            // 左馬達後退
#define IOCTL_LEFT_STOP         _IO(MOTOR_IOC_MAGIC, 4)            // 左馬達停止
#define IOCTL_RIGHT_FORWARD     _IO(MOTOR_IOC_MAGIC, 5)            // 右馬達前進
#define IOCTL_RIGHT_BACKWARD    _IO(MOTOR_IOC_MAGIC, 6)            // 右馬達後退
#define IOCTL_RIGHT_STOP        _IO(MOTOR_IOC_MAGIC, 7)            // 右馬達停止
#define IOCTL_BOTH_FORWARD      _IO(MOTOR_IOC_MAGIC, 8)            // 雙馬達前進
#define IOCTL_BOTH_BACKWARD     _IO(MOTOR_IOC_MAGIC, 9)            // 雙馬達後退
#define IOCTL_BOTH_STOP         _IO(MOTOR_IOC_MAGIC, 10)           // 雙馬達停止
#define IOCTL_TURN_LEFT         _IO(MOTOR_IOC_MAGIC, 11)           // 左轉 (左馬達反轉，右馬達前進)
#define IOCTL_TURN_RIGHT        _IO(MOTOR_IOC_MAGIC, 12)           // 右轉 (左馬達前進，右馬達反轉)
#define IOCTL_SET_PERIOD        _IOW(MOTOR_IOC_MAGIC, 13, unsigned int)

#endif