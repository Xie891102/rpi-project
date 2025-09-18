// UART 柱列 Queue
// 用於 tx/rx緩衝

#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include "uart_queue.h"

// 1.初始化
void queue_init(uart_queue_t *q){
	q->head = 0;	// 隊頭索引
	q->tail = 0;	// 隊尾索引
	q->count = 0;	// 當前元素數量
	
	// 初始化互斥鎖，保戶多執行緒存取
	pthread_mutex_init(&q->mutex, NULL);
	
	// 初始化條件變數，供thread等候或喚醒
	pthread_cond_init(&q->cond, NULL);
}



// 將資料傳入 queue
// 成功回傳0 失敗回傳-1(queue滿)
int queue_push(uart_queue_t *q, const char *data){
	
	// 上鎖，保護共享資源
	pthread_mutex_lock(&q->mutex);

	if(q->count >= UART_QUEUE_SIZE){
		// queue滿了
		pthread_mutex_unlock(&q->mutex);
		return -1;
	}

	// 複製字串到隊尾
	strncpy(q->data[q->tail], data, UART_DATA_MAX -1);
	q->data[q->tail][UART_DATA_MAX-1] = '\0';	// 確保字串結尾
	
	// 更新 tail 索引
	q->tail = (q->tail+1)%UART_QUEUE_SIZE;
	q->count++;

	// 喚醒可能在 queue_pop 等待的 thread
	pthread_cond_signal(&q->cond);

	// 解鎖
	pthread_mutex_unlock(&q->mutex);
	return 0;
}


// 從 queue 取資料(阻塞)
// 成功回傳0 失敗回傳-1
int queue_pop(uart_queue_t *q, char *buf, size_t buf_size){
	
	// 上鎖
	pthread_mutex_lock(&q->mutex); 

	// 如果queue空，等待cond被喚醒
	while(q->count == 0){
		pthread_cond_wait(&q->cond, &q->mutex);
	}

	// 取出隊頭資料
	strncpy(buf, q->data[q->head], buf_size-1);
	buf[buf_size-1] = '\0';

	// 更新 head 索引
	q->head = (q->head + 1)%UART_QUEUE_SIZE;
	q->count--;

	// 解鎖
	pthread_mutex_unlock(&q->mutex);
	return 0;
}


// 判斷 queue 是否為空
// 空回傳非0 否則回傳0
int queue_is_empty(uart_queue_t *q){
	
	int empty;
	pthread_mutex_lock(&q->mutex);
	empty = (q->count == 0);
	pthread_mutex_unlock(&q->mutex);
	return empty;
}


// 釋放queue的 mutex與cond
void queue_destroy(uart_queue_t *q){
	pthread_mutex_destroy(&q->mutex);
	pthread_cond_destroy(&q->cond);
}














