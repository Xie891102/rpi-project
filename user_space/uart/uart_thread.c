// uart_thread.c 執行緒標頭檔

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "uart.h"
#include "uart_queue.h"
#include "uart_thread.h"


// 全域變數
static uart_queue_t tx_queue;	// tx queue存放要發送的資料
static uart_queue_t rx_queue;	// rx queue存放接收到的資料
static pthread_t tx_thread;	// tx thread執行緒
static pthread_t rx_thread;	// rx thread執行緒
static volatile int stop_flag = 0;	// 停止執行緒旗標 1停止 0運行
static uart_rx_callback_t rx_callback = NULL;

// 註冊 callback
void uart_set_rx_callback(uart_rx_callback_t cb){
	rx_callback = cb;
}

// RX 執行緒函式(read)
static void* uart_rx_thread_func(void *arg){
	
	// 暫存從 uart 讀到的資料
	char buf[UART_DATA_MAX];

	while(!stop_flag){
		
		// 從 uart 讀資料
		int n = uart_read(buf, sizeof(buf)-1);
		if(n > 0){
			buf[n] = '\0';	// 確保字串結尾
			// 將獨到的資料放入 rx queue
			queue_push(&rx_queue, buf);		// 保留queue緩衝
			if(rx_callback) rx_callback(buf, n);	// 事件驅動
			
			//	pthread_cond_signal(&rx_queue.cond);
			
		} else {
			usleep(1000);	// 沒資料短暫休息，避免cpu空轉
		}
	}
	return NULL;

}


// TX 執行緒函式(write)
static void* uart_tx_thread_func(void *arg){
	
	// 儲存要發送的資料
	char buf[UART_DATA_MAX];
	
	while(!stop_flag){
		
		// 從 TX queue 取資料，如果 queue 空會阻塞等待 cond
		queue_pop(&tx_queue, buf, sizeof(buf));
		if(stop_flag) break;	// 停止旗標設置

		// 發送資料到 uart
		int n = uart_write(buf, strlen(buf));
		if(n < 0){
			perror("uart_write 失敗");
		} else {
			printf("UART 已發送: %s\n", buf);
		}
	}
	return NULL;
}	


// 啟動 UART 執行緒
int uart_thread_start(const char *device){
	
	// 初始化
	queue_init(&tx_queue);
	queue_init(&rx_queue);

	// 開啟UART
	if(uart_open(device) < 0) return -1;

	stop_flag = 0;	// 設定運行

	// 建立 TX 執行緒
	if(pthread_create(&tx_thread, NULL, uart_tx_thread_func, NULL) != 0){
		perror("TX thread 建立失敗");
		uart_close();
		return -1;
	}

	// 建立 RX 執行緒
	if(pthread_create(&rx_thread, NULL, uart_rx_thread_func, NULL) != 0){
		perror("RX thread 建立失敗");
		stop_flag = 1;
		pthread_cond_signal(&tx_queue.cond);	//  喚醒 TX thread
		pthread_join(tx_thread, NULL);
		uart_close();
		return -1;
	}
	return 0;
}


// 停止 UART 執行緒
void uart_thread_stop(){
	
	// 設置停止旗標
	stop_flag = 1;

	// 喚醒可能阻塞在 queue 的 thread
	pthread_cond_signal(&tx_queue.cond);
	pthread_cond_signal(&rx_queue.cond);

	// 等待 thread 安全退出
	pthread_join(tx_thread, NULL);
	pthread_join(rx_thread, NULL);

	// 關閉 UART
	uart_close();

	// 釋放queue 資源	
	queue_destroy(&tx_queue);
	queue_destroy(&rx_queue);
}


// Logic層呼叫: 非阻塞發送
int uart_send(const char *data){
	if(queue_push(&tx_queue, data) != 0){
		fprintf(stderr, "TX queue 已滿，無法發送資料: %s\n", data);
		return -1;
	}
	return 0;
}


// Logic層呼叫: 非阻塞接收
int uart_receive(char *buf, size_t buf_size){
	if(queue_is_empty(&rx_queue)) return 0;	// 沒有資料
	int ret = queue_pop(&rx_queue, buf, buf_size);
	printf("[DEBUG] uart_recieve pop success, queue count left= %d\n", rx_queue.count);
	return ret;
}





