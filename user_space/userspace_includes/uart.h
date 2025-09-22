// uart device 封裝標頭檔
#ifndef UART_H
#define UART_H

#include <stddef.h>

// 每次傳輸最大資料長度(和queue一樣)
#define UART_DATA_MAX 128

// 開啟 UART device 
int uart_open(const char *device);

// 關閉 UART device
int uart_close(void);

// 讀取 UART device (讀取到位元組數，<0 表示錯誤) 
// buf儲存讀到的資料  buf_size buf大小
int uart_read(char *buf, size_t buf_size);


// 寫入 UART device (寫入成功的位元組數，<0 表示錯誤) 
// data要送出的字串  len資料長度
int uart_write(const char *data, ssize_t len);


#endif 
