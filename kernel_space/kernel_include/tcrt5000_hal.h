// TCRT5000 hal 與 driver間使用的(避免read_gpio()編譯時未讀取到定義)

#ifndef __TCRT5000_HAL_H__
#define __TCRT5000_HAL_H__

int read_gpio(int gpio);

#endif


