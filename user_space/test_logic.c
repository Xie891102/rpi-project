// 測試邏輯層
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

// 自訂標頭檔
#include "hcsr04.h"  		// 超聲波(避障功能)
#include "buzzer.h"		// 蜂鳴器
#include "tcrt5000.h"		// 紅外線(循跡)
#include "motor_ctrl.h"		// 馬達控制器

#include "route.h"			// 路線解析
#include "mqtt_config.h"		// 無線通訊(調度中心Rpi)
#include "uart.h"			// 有線通訊 open/release(開/關檔案 <-> pico)
#include "uart_thread.h"		// 有線通訊 read/write(非阻塞讀/寫 <-> pico)

// 動作對照
#define  STRAIGHT	0
#define  LEFT 	1	
#define  RIGHT	2
#define  STOP	3

// 馬達速度設定
#define SPEED_INIT   40    		// 基本速度百分比
#define SPEED1 	 3     		// 微調整
#define SPEED2 	 6    		// 中等調整
#define SPEED3 	10      		// 大調整

bool node_active = false;  		// 碰到節點時紅外線鎖定旗標
int stop_flag = 0;         	// 控制 thread 停止
tcrt5000_callback logic_cb = NULL;	// callback 指標 (thread 會呼叫)

int nodes[] = {STRAIGHT};
int total_nodes = sizeof(nodes)/sizeof(nodes[0]);
int current_index = 0;


//  --------------------------------------- 讀取節點 --------------------------------------------------
void handle_node(int next_action) {
    
	printf("[NODE] 節點處理開始\n");

	// 1.先降到基礎速度
	set_left_motor(SPEED_INIT, 1);
	set_right_motor(SPEED_INIT, 1);
	sleep(1);


	// 2.根據下一個動作處理 
    	switch (next_action) {
        		// 直行指令
		case STRAIGHT:
            			move_forward();
			printf("[NODE] 節點直行\n");
            			break;
        
		// 左轉指令
		case LEFT:
            			turn_left();	// 馬達左轉
			sleep(1);		// 緩衝
			move_forward();	// 轉彎後直行
			printf("[NODE] 節點左轉\n");
	            		break;
        
		// 右轉指令
		case RIGHT:
            			turn_right();	// 馬達右轉
			sleep(1);		// 緩衝
			move_forward();	// 轉彎後直行
			printf("[NODE] 節點右轉\n");
            			break;
        
		// 停車
		case STOP:
            			stop_all_motors();
			printf("[NODE] 節點停車\n");
            			break;
			
		// 其他
        		default:
            			move_forward();
			printf("[NODE] 未知動作\n");
            			break;
    	}
}


// ----------------------------------------- 循跡tcrt5000  -------------------------------------------------
void logic(int code) {
	switch(code) {
		case 0:	// 000 => 出軌	
			stop_all_motors();
			printf("[LOGIC] \n");
			break;
	
		case 2:	// 010 => 正常行駛	
			move_forward();
			printf("[LOGIC] 正常行駛，繼續前進\n");
			break;

		case 1:	// 001 => 太左偏
			set_left_motor(SPEED_INIT + SPEED2, 1);
			printf("[LOGIC] 太左偏，強力往右修正\n");
			break;

		case 3:	// 011 => 微左偏
			set_left_motor(SPEED_INIT + SPEED1, 1);
			printf("[LOGIC] 微左偏， 左輪微加速\n");
			break;
		
        		case 4: 	// 100 => 太右偏
		            	set_right_motor(SPEED_INIT + SPEED2, 1);  
			printf("[LOGIC] 太右偏，強力往左修正\n");
            			break;
	
       		 case 6: 	// 110 => 微右偏
                		set_right_motor(SPEED_INIT + SPEED1, 1);
            			printf("[LOGIC] 微右偏， 右輪微加速\n");
            			break;
		
        		case 5: 	// 101 => 目標位置 (終點)
            			printf("[LOGIC] 到達終點，準備停車(3秒後)\n");
			stop_all_motors();   	// 停車
			sleep(3);            		// 延遲3秒，避免慣性
			break;
		
        		case 7: 	// 111 => 節點
			if(!node_active && current_index < total_nodes){
				node_active = true;       		// 鎖定節點
				handle_node(nodes[current_index]);   // 調用讀取節點函式
				current_index++;
				printf("[LOGIC] 碰到節點\n");
				node_active = false;      		// 轉彎完成解鎖
			}		
			break;   
			
		default:	// 其他: 判斷不出來
            			printf("[LOGIC] 未知編碼: %d\n", code);
            			break;
	}
}



// ------------------- 主程式 -------------------
int main() {
    	printf("[MAIN] 測試開始\n");

	// 開啟紅外線裝置
    	if (tcrt5000_open() < 0) {
        	printf("無法開啟紅外線裝置，停止測試\n");
        	return 1;
    	}

    	pthread_t tcrt_thread;

    	// 設定 callback
    	logic_cb = logic;

    	// 建立紅外線循跡 thread
    	pthread_create(&tcrt_thread, NULL, tcrt5000_thread_func, NULL);

    	// 等待 thread 完成 (或在測試中可以用 sleep 模擬)
    	// 這裡假設 thread 自己會根據 stop_flag 停止
    	while(current_index < total_nodes) {
        		usleep(50000);  // 50ms，避免 busy loop
    	}

    	// 停止 thread
    	stop_flag = 1;
    	pthread_join(tcrt_thread, NULL);

    	printf("[MAIN] 測試路線完成\n");
    	return 0;
}

