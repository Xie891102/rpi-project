#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

// 自訂標頭檔
#include "hcsr04.h"  			// 超聲波(避障功能)
#include "buzzer.h"			// 蜂鳴器
#include "tcrt5000.h"			// 紅外線(循跡)
#include "motor_ctrl.h"			// 馬達控制器

#include "route.h"			// 路線解析
#include "mqtt_config.h"		// 無線通訊(調度中心Rpi)
#include "uart.h"			// 有線通訊 open/release(開/關檔案 <-> pico)
#include "uart_thread.h"		// 有線通訊 read/write(非阻塞讀/寫 <-> pico)


// 馬達速度設定
#define SPEED_INIT   40    	// 基本速度百分比
#define SPEED1 	 3     		// 微調整
#define SPEED2 	 6    		// 中等調整
#define SPEED3 	10      	// 大調整

bool node_active = false;  	// 碰到節點時紅外線鎖定旗標


// 超聲波避障功能
void emergency_stop() {
	
    	// 1. 停止馬達
    	stop_all_motors();

    	// 2. 解鎖節點，避免後續邏輯被鎖死
    	node_active = false;

    	// 3. MQTT 通知調度中心
    	char msg[128];
    	sprintf(msg, "{\"status\":\"stuck\"}");
    	mqtt_publish(MQTT_TOPIC_CAR, msg);

    	// 4. UART 發紅燈
    	uart_send("D");

    	// 5. 蜂鳴器叫
    	buzzer_write(1);
}


// 解除超聲波
void emergency_clear() {
	
    	// 1. 蜂鳴器關閉
    	buzzer_write(0);

    	// 2. 紅燈關閉
   	uart_send("d");  // 假設小寫 d 代表紅燈熄滅
	
	// 3. 解鎖節點，避免後續邏輯被鎖死
    	node_active = false;
	
	// 4. MQTT 通知調度中心
    	char msg[128];
    	sprintf(msg, "{\"status\":\"restart\"}");
    	mqtt_publish(MQTT_TOPIC_CAR, msg);

}


// 超聲波資料 callback 函式
// 由 hcsr04_thread_func 執行緒呼叫，每次讀取到新的距離就會觸發
void hcsr04_callback(hcsr04_all_data *data) {
	// 靜態計數器，用來累計連續距離過近的次數
    	static int cnt = 0;

    	// 檢查前方超聲波距離是否有效且小於 5 公分
    	if(data->ultrasonic[0].distance > 0 && data->ultrasonic[0].distance < 5) {
        	cnt++;  // 連續危險計數累加

        	// 如果連續三次小於 5 公分，就觸發緊急停止
        	if(cnt >= 3) {
            		printf("[EMERGENCY] 前方障礙物 <5cm，緊急停止\n");

	            	// 統一處理緊急停止：停車、蜂鳴器、紅燈、MQTT 通知
            		emergency_stop();

            		cnt = 0;  // 重置計數器
        	}
    	} else {
        	// 如果距離安全，重置計數器
        	cnt = 0;
    	}
}



// 讀取節點
void handle_node(Route *route) {
    
	// 1.檢查是否有值
	if (!route) return;

    	// 2.檢查是否有下一步
    	if (!has_next(route)) return;
	
	// 3.取得下一步
    	Action next = next_step(route);
    	printf("[ROUTE] 下一步: %s\n", action_to_string(next));
	
	// 4. 發送 MQTT 訊息
	int current_node = route->current;  // next_step 已經自動 +1
	char msg[128];
	sprintf(msg, "{\"node\":%d,\"doing\":\"%s\"}", current_node, action_to_string(next));
	mqtt_publish(MQTT_TOPIC_CAR, msg);

    	// 5.先降到基礎速度
	set_left_motor(SPEED_INIT, 1);
	set_right_motor(SPEED_INIT, 1);
    
	// 6. 延遲2秒後再執行判斷
	sleep(1);        

    	// 7.執行指令
    	switch (next) {
        	// 直行指令
		case STRAIGHT:
            		move_forward();
            		break;
        
		// 左轉指令
		case LEFT:
			uart_send("L");  	// 左轉燈亮(uart->pico)
            		turn_left();		// 馬達左轉
			sleep(1);		// 緩衝
			move_forward();		// 轉彎後直行
			uart_send("l");  	// 關閉左轉燈亮(uart->pico)
            		break;
        
		// 右轉指令
		case RIGHT:
			uart_send("R");  	// 右轉燈亮(uart->pico)
            		turn_right();		// 馬達右轉
			sleep(1);		// 緩衝
			move_forward();		// 轉彎後直行
			uart_send("r");  	// 關閉右轉燈亮(uart->pico)
            		break;
        
		// 停車
		case STOP:
            		stop_all_motors();
            		break;
			
		// 其他
        	default:
            		move_forward();
            		break;
    	}
}




// 邏輯函式
void logic(int code) { 

    switch(code) {
	
	// 000 => 出軌
        case 0:
            	// 停止馬達 
            	stop_all_motors();
                printf("[LOGIC] 出軌！停車並啟動尋線模式\n");
				
		// 通知pico閃紅燈
		uart_send("D");  
				
		// 通知調度中心(mqtt)
		char msg[128];
		sprintf(msg, "{\"status\":\"off_track\"}");
		mqtt_publish(MQTT_TOPIC_CAR, msg);
                break;


	// 010 => 正常在線上
        case 2: 
            	// 前進，雙輪等速
            	move_forward();
            	printf("[LOGIC] 正常行駛，繼續前進\n");
            	break;


	// 001 => 太左偏
        case 1: 
		// 左輪調快
		set_left_motor(SPEED_INIT + SPEED2, 1);  	
           	printf("[LOGIC] 太左偏，強力往右修正\n");           		
            	break;


	// 011 => 微左偏
        case 3: 
                // 左輪稍加速
                set_left_motor(SPEED_INIT + SPEED1, 1);            		
		printf("[LOGIC] 微左偏， 左輪微加速\n");
            	break;


	// 100 => 太右偏
        case 4: 
		// 右輪調快
            	set_right_motor(SPEED_INIT + SPEED2, 1);  
		printf("[LOGIC] 太右偏，強力往左修正\n");
            	break;


	// 110 => 微右偏
        case 6: 
               	// 右輪稍加速
                set_right_motor(SPEED_INIT + SPEED1, 1);
            	printf("[LOGIC] 微右偏， 右輪微加速\n");
            	break;


	// 101 => 目標位置 (終點)
        case 5: 
            	printf("[LOGIC] 到達終點，準備停車(3秒後)\n");
			stop_all_motors();   // 停車
			sleep(3);            // 延遲3秒，避免慣性

			// 通知調度中心
			sprintf(msg, "{\"status\":\"arrived\"}");
			mqtt_publish(MQTT_TOPIC_CAR, msg);
				
			// 根據節點最後一碼類型啟動輸送帶
			if(route->current_node_type == 1){ 	
				// 1 = 送貨
				uart_send("S");   // 啟動輸送帶(uart->pico)
					
				// 通報調度中心(送出貨物中)
				sprintf(msg, "{\"status\":\"moving\",\"delivery_status\":\"delivering\"}");
				mqtt_publish(MQTT_TOPIC_CAR, msg);
			} else {			
				// 0 = 接貨
				
				// 通報調度中心(接收貨物中)
				sprintf(msg, "{\"status\":\"moving\",\"delivery_status\":\"receiving\"}");
				mqtt_publish(MQTT_TOPIC_CAR, msg);
			}
				
			// 通知調度中心(任務結束)
			sprintf(msg, "{\"status\":\"arrived\",\"delivery_status\":\"completed\"}");
			mqtt_publish(MQTT_TOPIC_CAR, msg);
			break;


	// 111 => 節點
        case 7: 
		if(!node_active){
			node_active = true;       		// 鎖定節點
			handle_node(route);			// 調用讀取節點函式
			printf("[LOGIC] 碰到節點\n");
			node_active = false;      		// 轉彎完成解鎖
		}
				
		break;   
     

	// 其他: 判斷不出來
		default:
            	printf("[LOGIC] 未知編碼: %d\n", code);
            	break;
    }
}









