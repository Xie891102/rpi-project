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

