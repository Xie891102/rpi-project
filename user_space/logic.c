#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "hcsr04.h"  		// 超聲波(避障功能)
#include "buzzer.h"		// 蜂鳴器
#include "tcrt5000.h"		// 紅外線(循跡)
#include "motor_ctrl.h"		// 馬達控制器

#include "route.h"		// 路線解析
#include "mqtt_config.h"	// 無線通訊(調度中心Rpi)
#include "uart.h"		// 有線通訊 open/release(開/關檔案 <-> pico)
#include "uart_thread.h"	// 有線通訊 read/write(非阻塞讀/寫 <-> pico)


// 外部全域變數
extern int stop_flag;           	// 停止旗標，由 main 控制
extern tcrt5000_callback logic_cb; 	// callback 指標，由 logic.c 定義
extern Route *route;			// 全域路線指標

static int adjust_count = 0;  		// 記錄連續微調次數

// 避障距離 (公分)
#define SAFE_DISTANCE 	50		// 50公分

// 馬達速度設定
#define BASE_SPEED      40      	// 基本速度百分比

#define DELTA_MICRO 	10     		// 微調整
#define DELTA_MEDIUM 	25    		// 中等調整
#define DELTA_BIG 	40      		 // 大調整

// 節點計數
static int current_node = 0;



// ---------------  節點處理函式  ---------------
void handle_node() {
    if(!route) return;

    // 1. 更新節點計數
    current_node++;
    printf("[NODE] 到達節點 %d\n", current_node);

    // 2. 回報節點給 RPi
    char msg[64];
    snprintf(msg, sizeof(msg), "NODE:%d", current_node);
    mqtt_publish(MQTT_TOPIC_CAR, msg);

    // 3. 延遲 3 秒
    sleep(3);

    // 4. 判斷下一步
    if(has_next(route)) {
        Action act = next_step(route);
        printf("[NODE] 下一步動作: %s\n", action_to_string(act));

        // 發布下一步動作
        snprintf(msg, sizeof(msg), "ACTION:%s", action_to_string(act));
        mqtt_publish(MQTT_TOPIC_CAR, msg);

        // 5. 執行動作
        switch(act) {
            case STRAIGHT: move_forward(); break;
            case LEFT:     turn_left();    break;
            case RIGHT:    turn_right();   break;
            case STOP:     stop_all_motors(); break;
        }
    } else {
        // 沒有下一步就停車
        stop_all_motors();
        printf("[NODE] 路線結束，已停車\n");
        mqtt_publish(MQTT_TOPIC_CAR, "FINISHED");
    }
}




// ---------------  Callback function  ---------------
void logic_cb(int code) {
    switch(code) {
	
	// 000 => 完全出軌
        case 0:
            	// 停止馬達
            	       	stop_all_motors();
                      	// 通報調度中心：出軌
                     	report_status("offtrack");   
                     	printf("[LOGIC] 出軌！停車並啟動尋線模式\n");
                     	break;

	// 010 => 正常在線
        case 2: 
            	// 前進，雙輪等速
            		move_forward();
            		// 微調累計歸零
            		adjust_count = 0; 
            		printf("[LOGIC] 正常在線，前進\n");
            		break;

	// 001 => 太右偏
        	case 1: 
            		printf("[LOGIC] 太右偏，強力往左修正\n");
            		set_left_motor(BASE_SPEED - DELTA_BIG, 1);  // 左輪慢
            		set_right_motor(BASE_SPEED + DELTA_BIG, 1); // 右輪快
            		break;

	// 011 => 微右偏
        case 3: 
            	adjust_count++;
            	{
                	int delta = (adjust_count >= 3) ? DELTA_MEDIUM : DELTA_MICRO;
                	// 左輪略慢、右輪略快，往左修正
                		set_left_motor(BASE_SPEED - delta, 1);
               	 		set_right_motor(BASE_SPEED + delta, 1);
            		}
           	 	printf("[LOGIC] 微右偏，左修正，微調次數 %d\n", adjust_count);
            		break;

	// 100 => 太左偏
        case 4: 
            	printf("[LOGIC] 太左偏，強力往右修正\n");
            	set_left_motor(BASE_SPEED + DELTA_BIG, 1);   // 左輪快
            	set_right_motor(BASE_SPEED - DELTA_BIG, 1);  // 右輪慢
            	break;

	// 110 => 微左偏
        case 6: 
		// 累計微調次數
            	adjust_count++; 
            		{
                		// 若微左偏連續 >= 3 次，使用中等速差
                		int delta = (adjust_count >= 3) ? DELTA_MEDIUM : DELTA_MICRO;
               		 	// 左輪略快、右輪略慢，往右修正
                		set_left_motor(BASE_SPEED + delta, 1);
                		set_right_motor(BASE_SPEED - delta, 1);
          	 	 }
            		printf("[LOGIC] 微左偏，右修正，微調次數 %d\n", adjust_count);
            		break;

	// 101 => 目標位置 (終點)
        case 5: 
            	printf("[LOGIC] 到達終點，延遲 3 秒停車\n");
            		sleep(3);            // 延遲 3 秒
            		stop_all_motors();   // 停車
            		report_status("finish"); // 通報調度中心
            		break;

	// 111 => 節點
        case 7: 
            	// 節點計數累加
    		current_node++;

    		printf("[LOGIC] 節點 %d，呼叫節點處理函式\n", current_node);

    		// 呼叫節點處理函式 (延遲、通報、下一步判斷)
   		 handle_node(current_node);
    		break;   
     
	// 其他: 判斷不出來
	default:
            	printf("[LOGIC] 未知編碼: %d\n", code);
            	break;
    }
}




