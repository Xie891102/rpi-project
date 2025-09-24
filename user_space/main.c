// 車輛入口應用程式

#include <stdio.h>      	// printf, fprintf
#include <stdlib.h>     	// exit, NULL
#include <unistd.h>     	// sleep
#include <pthread.h>    	// pthread_create, pthread_join
#include <signal.h>     	// signal, SIGINT

#include "tcrt5000.h"		// 循跡感測器 API
#include "hcsr04.h"      	// 超聲波感測器 API
#include "logic.h"      	// logic_cb callback 宣告
#include "motor_ctrl.h" 	// stop_all_motors() 與馬達控制
#include "mqtt_config.h"	// 無線通訊
#include "route.h"		// 路線解析


// ---------------- 全域變數 ----------------
int stop_flag = 1;                 // 1 = 停止, 0 = 運行
bool node_active = false;          // 節點鎖定旗標
Route *my_route = NULL;            // 存放解析後路線

pthread_t tcrt_thread;
pthread_t hcsr_thread;


// ---------------- 初始化系統 ----------------
void initialize_system() {
    	// 1. 停止馬達
    	stop_all_motors();

    	// 2. 開啟循跡感測器
    	if (tcrt5000_open() != 0) {
        		fprintf(stderr, "無法開啟 TCRT5000\n");
        		exit(-1);
    	}

    	// 3. 設定紅外線 callback
    	logic_cb = logic;

    	// 4. 啟動循跡 thread
    	if(pthread_create(&tcrt_thread, NULL, tcrt5000_thread_func, NULL) != 0) {
        		fprintf(stderr, "無法建立循跡 thread\n");
        		exit(-1);
    	}

    	// 5. 設定超聲波 callback
    	distance_cb = hcsr04_callback;
    	if(pthread_create(&hcsr_thread, NULL, hcsr04_thread_func, NULL) != 0) {
        		fprintf(stderr, "無法建立超聲波 thread\n");
        		exit(-1);
    	}

    	// 6. 初始化 MQTT
    	mqtt_init();
    	mqtt_subscribe(MQTT_TOPIC_CAR, mqtt_message_callback);
}


// ---------------- MQTT callback ----------------
void mqtt_message_callback(const char *topic, const char *payload) {
    	if (strcmp(topic, MQTT_TOPIC_CAR) != 0) return;

	// 1. 收到開始訊號
    	if (strstr(payload, "\"start\":1")) {
        		printf("[MQTT] 收到開始訊號\n");
        		if(my_route != NULL) {
            			reset_route(my_route);
            			stop_flag = 0;
            			node_active = false;
        		} else {
            			printf("[MQTT] 尚未有路線資料\n");
        		}

	// 2. 收到停止訊號
    	}  else if (strstr(payload, "\"stop\":1")) {
        		printf("[MQTT] 收到停止訊號\n");
        		stop_flag = 1;
	        	stop_all_motors();

	// 3. 收到路線資料
    	} else if (strstr(payload, "\"route\":")) {
        	printf("[MQTT] 收到路線資料\n");

    		int raw_steps[15];   // 最多 15 步
    		int count = 0;

   		// 找到 "route":[ 開始解析
    		const char *p = strstr(payload, "\"route\":[") + 9;
    		while(*p && *p != ']' && count < 15) {
       			if(*p >= '1' && *p <= '4') {
           			raw_steps[count++] = *p - '0';  // 字元轉數字
       	 		}
        		p++;
   	 	}

    		if(count > 0) {
        		if(my_route) free_route(my_route);
        		my_route = create_route(raw_steps, count);
        		printf("[MQTT] 路線解析完成，步驟數: %d\n", count);

			// 判定最後一步送貨/接貨
            		if (strstr(payload, "\"delivery\":1")) {
                		my_route->node_type[count-1] = 1; // 送貨
            		} else {
                		my_route->node_type[count-1] = 2; // 接貨
            		}

            		printf("[MQTT] 路線解析完成，步驟數: %d\n", count);

    		} else {
        		printf("[MQTT] 沒有有效路線步驟\n");
    		}

    	 // 4. 收到關閉蜂鳴器訊號
	} else if (strstr(payload, "\"buzzer_off\":1")) {
        		printf("[MQTT] 收到關閉蜂鳴器訊號\n");
        		emergency_clear();
    	}
}


// ---------------- 停止系統 ----------------
void shutdown_system() {
    	stop_flag = 1;
    	pthread_join(tcrt_thread, NULL);
    	pthread_join(hcsr_thread, NULL);

    	tcrt5000_close();
    	stop_all_motors();

    	if(my_route) free_route(my_route);
    	mqtt_close();
}

// ---------------- 主迴圈 ----------------
void main_loop() {
    	while(1) {
        		char cmd;
        		printf("\n輸入指令 (0=立即停止, 1=開始運行, 3=結束程式, 4=解除緊急): ");
        		scanf(" %c", &cmd);

		// 1.手動停止，未結束程式
        		if(cmd == '0') {
            			printf("立即停止\n");
            			stop_flag = 1;
            			stop_all_motors();
		
		// 2.手動開始
        		} else if(cmd == '1') {
            			if(my_route != NULL) {
        		        		printf("手動開始運行\n");
                			reset_route(my_route);
                			stop_flag = 0;
                			node_active = false;
            			} else {
                			printf("尚未有路線資料\n");
           	 		}
		
		// 3.結束整個程式
        		} else if(cmd == '3') {
            			printf("結束程式\n");
            			break;

		// 4.解除超生坡問題
        		} else if(cmd == '4') {
            			printf("解除緊急狀態\n");
            			emergency_clear();
		
		// 5.其他
        		} else {
            			printf("未知指令\n");
        		}

        		usleep(100000); // 避免 CPU 空轉
    	}
}


// ---------------- main ----------------
int main() {
    	printf("=== 車輛程式啟動 ===\n");

	// 1.初始化感測器、MQTT、thread
    	initialize_system();  

	// 2.主迴圈 (手動指令)
    	main_loop();           

	// 3.清理資源
    	shutdown_system();    

    	printf("程式已結束\n");
    	return 0;
}


