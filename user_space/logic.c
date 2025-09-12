// 行車邏輯層

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "sensors/tcrt5000.h"


// 停止旗標(全域變數在main.c檔案中)
extern int stop_flag;


// 循跡紅外線執行續
void* tcrt5000_thread_func(void* arg){
	
	tcrt5000_data sensor;

	// 停止條件
	while(!stop_flag){
		
		// 讀取數值
		if(tcrt5000_read(&sensor) == 0){
		
			// 二進位 轉 十進位	
			int code = sensor.left*4 + sensor.middle*2 + sensor.right*1;

			// 循跡條件判斷
			switch(code){
				case 0: printf("000 -> 狀態: 出軌 	     調整: 立即停車\n"); 		break;	
				case 2: printf("010 -> 狀態: 直線行駛 	     調整: 馬達保持轉速\n"); 		break;
				case 3: printf("011 -> 狀態: 車體微右偏了    調整: 微左調(右邊馬達提速1)\n"); 	break;
				case 1: printf("001 -> 狀態: 右偏了 	     調整: 左調(右邊馬達提速2)\n"); 	break;
				case 6: printf("110 -> 狀態: 微左偏 	     調整: 微右調(左邊馬達提速1)\n"); 	break;
				case 4: printf("100 -> 狀態: 左偏了 	     調整: 右調(左邊馬達提速2)\n"); 	break;
				case 7: printf("111 -> 狀態: 到達目的地      調整: 立即停車\n"); 		break;
				default: printf("其他狀態\n"); 							break;
			}
		} else {
			fprintf(stderr, "讀取失敗\n");	// 無法讀取時
		}
		
		usleep(500000);	// 0.5秒讀一次
	}
	return NULL;
}








