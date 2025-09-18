// Queue 標頭檔(uart_thread.c使用)  
#ifndef UART_QUEUE_H
#define UART_QUEUE_H

#include <pthread.h>
#include "uart.h"

// 最大柱列長度
#define UART_QUEUE_SIZE 32 


// queue 結構體
typedef struct {
	char data[UART_QUEUE_SIZE][UART_DATA_MAX];	// 存放字串資料
	int head;	// 隊頭索引
	int tail;	// 隊尾索引
	int count;	// 當前元素數量
	pthread_mutex_t mutex;	// 互斥鎖，保護多thread 同時存取
	pthread_cond_t cond;	// 條件變數，通知 TX thread 有新資料
} uart_queue_t;

// 初始化 queue
void queue_init(uart_queue_t *q);

// 放入資料 push
int queue_push(uart_queue_t *q, const char *data);

// 取出資料 pop
int queue_pop(uart_queue_t *q, char *buf, size_t buf_size);

// 判斷 queue 是否為空
int queue_is_empty(uart_queue_t *q);

// 釋放資源 
void queue_destroy(uart_queue_t *q);

#endif

