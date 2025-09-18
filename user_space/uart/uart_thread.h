// 執行緒標頭檔(logic.c呼叫使用)
#ifndef UART_THREAD_H
#define UART_THREAD_H

#include <stddef.h>	// size_t


// 啟動uart thread
int uart_thread_start(const char *device);


// 停止 uart thread
void uart_thread_stop(void);


// 非阻塞發送資料到uart 
// data要傳送的字串  0表示成功 -1表示queue滿了  
int uart_send(const char *data);


// 非阻塞接收uart資料	
// buf儲存接收到的資料  0表示沒有資料可讀  >0表示成功讀到資料了
int uart_receive(char *buf, size_t buf_size);



#endif


