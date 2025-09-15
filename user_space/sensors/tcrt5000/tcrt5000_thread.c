// 行車邏輯層

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "includes.h"


// 停止旗標 (main.c 定義)
extern int stop_flag;


// callback 指標 (logic.c 定義)
extern tcrt5000_callback logic_cb;


// 循跡紅外線執行續
void* tcrt5000_thread_func(void* arg){
	
	tcrt5000_data sensor;

	// 停止條件
	while(!stop_flag){
		
		// 1.讀取感測器
		if(tcrt5000_read(&sensor) == 0){
			
			// 將3個感測器組成一個 code(二進位轉十進未)
			int code = sensor.left*4 + sensor.middle*2 + sensor.right*1;

			// 呼叫 logic 的 callback 函式處裡
			if(logic_cb != NULL) {
				logic_cb(code);
			}
			
		} else {
			fprintf(stderr, "TCRT5000 讀取失敗\n");
		}	

		// 2.控制循環讀取的平率 (50~100 ms)
		usleep(50000);	// 50ms
	}

	return NULL;
}





