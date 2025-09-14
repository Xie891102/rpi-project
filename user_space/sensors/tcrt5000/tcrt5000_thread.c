// 行車邏輯層

#include <stdio.h>
#include <pthread.h>
#include "includes.h"


// 停止旗標 (main.c 定義)
extern int stop_flag;

// mutex / cond (main.c定義)
extern pthread_mutex_t tcrt_mutex;
extern pthread_cond_t tcrt_cond;

// callback 指標 (logic.c 定義)
extern tcrt5000_callback logic_cb;


// 循跡紅外線執行續
void* tcrt5000_thread_func(void* arg){
	
	tcrt5000_data sensor;

	// 停止條件
	while(1){
		
		// 1.等待 被logic_controller 喚醒
		pthread_mutex_lock(&tcrt_mutex);
		pthread_cond_wait(&tcrt_cond, &tcrt_mutex);

		// 2.檢查 stop_flag 是否要退出
		if(stop_flag){

			pthread_mutex_unlock(&tcrt_mutex);
			break;		// 結束thread
		}


		// 3.讀感測器
		if(tcrt5000_read(&sensor) == 0) {
			int code = sensor.left*4 + sensor.middle*2 + sensor.right*1;

			// 將資料回傳給 logic
			if(logic_cb != NULL) logic_cb(code);	// 呼叫 logic.c函式處理
			
		} else {
			fprintf(stderr, "TCRT5000 讀取失敗\n");
		}
	}

	return NULL;
}
