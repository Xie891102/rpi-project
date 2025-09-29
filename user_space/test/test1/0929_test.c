#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/time.h>
#include <string.h>

#include "tcrt5000.h"        // 紅外線循跡感測器模組
#include "motor_ctrl.h"      // 馬達控制器

// --------------------- 設定參數 ---------------------
#define DERAIL_SET_COUNT 6       // 判定出軌需連續偵測次數
#define DERAIL_SET_TIME 1000     // 判定出軌需持續時間(毫秒)
#define SPEED_INIT 40            // 馬達初始速度
#define SPEED1 5               // 微調修正幅度
#define SPEED2 10              // 大幅修正幅度
#define STATE_HISTORY 10         // 紀錄最近狀態數量
#define LINEAR_CHECK_N 5         // 判斷線性偏移時檢查最近 N 個狀態

// --------------------- 全域變數 ---------------------
static int derail_count = 0;       // 出軌累計次數
static long derail_time = 0;       // 出軌累計起始時間
volatile int stop_flag = 0;        // 停止旗標，用於結束主迴圈

int last_codes[STATE_HISTORY];     // 最近 STATE_HISTORY 次感測器狀態
int history_idx = 0;               // 紀錄陣列索引，循環使用

bool node_active = false;
tcrt5000_callback logic_cb = NULL;


// --------------------- 工具函數 ---------------------

// 取得系統目前毫秒數
long get_millis(void){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

// 找最近一次非出軌狀態，避免出軌誤判
int find_last_valid(){
    for(int i=0;i<STATE_HISTORY;i++){
        int idx = (history_idx - 1 - i + STATE_HISTORY) % STATE_HISTORY; // 循環索引
        int code = last_codes[idx];
        if(code != 0) return code; // 0表示出軌，其它都是有效狀態
    }
    return 2; // 如果全部是出軌，預設回線上
}

// 判斷最近 N 個歷史狀態是否有線性偏移趨勢
bool is_linear_shift(){
    int count = 0;
    for(int i=1;i<=LINEAR_CHECK_N;i++){
        int idx = (history_idx - i + STATE_HISTORY) % STATE_HISTORY;
        int s = last_codes[idx];
        if(s==1 || s==3 || s==4 || s==6){ // 偏左或偏右
            count++;
        }
    }
    return count > 0; // 最近有偏移趨勢
}

// --------------------- switch case 動作函數 ---------------------

/*
 * 根據 code 狀態控制馬達行為
 * 0：出軌
 * 1：太左偏
 * 2：正常線上
 * 3：微左偏
 * 4：太右偏
 * 5：終點
 * 6：微右偏
 * 7：節點
 */
void handle_state(int code){
    switch(code){
        case 0: // 出軌
            if(is_linear_shift()){
                // 如果最近有偏移趨勢 → 0 是偏移造成的，立即拉回線上
                int restore = find_last_valid();
                handle_state(restore); // 遞迴呼叫恢復前一個有效狀態
                derail_count = 0;
                derail_time = 0;
				printf("[SIM] 出軌偏移恢復到狀態 %d\n", restore);
            } else {
                // 最近沒有偏移趨勢 → 可能誤判，累計出軌
                if(derail_count==0) derail_time = get_millis(); // 記錄起始時間
                derail_count++;

                // 判斷是否達到出軌停車條件
                if(derail_count >= DERAIL_SET_COUNT &&
                    get_millis() - derail_time >= DERAIL_SET_TIME){
                    stop_all_motors();
                    printf("[SIM] 出軌停車 (derail_count=%d)\n", derail_count);
					// printf("[LOGIC] 出軌！停車\n");
                    derail_count = 0;
                    derail_time = 0;
                }
            }
            break;

        case 1: // 太左偏
            // set_left_motor(SPEED_INIT - SPEED1,1);   // 左輪減速
            // set_right_motor(SPEED_INIT + SPEED2,1);  // 右輪加速
            printf("[SIM] 太左偏 -> 左輪:%d, 右輪:%d\n", SPEED_INIT - SPEED1, SPEED_INIT + SPEED2);
			derail_count = 0; derail_time = 0;
            break;

        case 2: // 正常線上
            // set_left_motor(SPEED_INIT,1);
            // set_right_motor(SPEED_INIT,1);
            printf("[SIM] 正常線上 -> 左輪:%d, 右輪:%d\n", SPEED_INIT, SPEED_INIT);
			derail_count = 0; derail_time = 0;
            break;

        case 3: // 微左偏
            // set_left_motor(SPEED_INIT - SPEED1,1);
            // set_right_motor(SPEED_INIT + SPEED1,1);
            printf("[SIM] 微左偏 -> 左輪:%d, 右輪:%d\n", SPEED_INIT - SPEED1, SPEED_INIT + SPEED1);
			derail_count = 0; derail_time = 0;
            break;

        case 4: // 太右偏
            // set_left_motor(SPEED_INIT + SPEED2,1);
            // set_right_motor(SPEED_INIT - SPEED2,1);
			printf("[SIM] 太右偏 -> 左輪:%d, 右輪:%d\n", SPEED_INIT + SPEED2, SPEED_INIT - SPEED2);
            derail_count = 0; derail_time = 0;
            break;

        case 5: // 終點
            // stop_all_motors();
            // printf("[INFO] 到達終點\n");
			printf("[SIM] 到達終點，停止馬達\n");
            stop_flag = 1;
            break;

        case 6: // 微右偏
            // set_left_motor(SPEED_INIT + SPEED1,1);
            // set_right_motor(SPEED_INIT - SPEED1,1);
			printf("[SIM] 微右偏 -> 左輪:%d, 右輪:%d\n", SPEED_INIT + SPEED1, SPEED_INIT - SPEED1);
            derail_count = 0; derail_time = 0;
            break;

        case 7: // 節點
            // set_left_motor(SPEED_INIT,1);
            // set_right_motor(SPEED_INIT,1);
			printf("[SIM] 節點 -> 左輪:%d, 右輪:%d\n", SPEED_INIT, SPEED_INIT);
            derail_count = 0; derail_time = 0;
            break;

        default: // 其他未知狀態
            // set_left_motor(SPEED_INIT,1);
            // set_right_motor(SPEED_INIT,1);
			printf("[SIM] 未知狀態 -> 左輪:%d, 右輪:%d\n", SPEED_INIT, SPEED_INIT);
            derail_count = 0; derail_time = 0;
            break;
    }
}

// --------------------- 主回調函數 ---------------------

/*
 * 每次感測器回傳狀態時呼叫
 * 1. 紀錄最近 STATE_HISTORY 個狀態
 * 2. 呼叫 handle_state 控制馬達
 */
void logic(int code){
    last_codes[history_idx++] = code;             // 更新歷史狀態
    if(history_idx>=STATE_HISTORY) history_idx=0; // 循環索引
	
	printf("[SIM] 當前狀態: %d, 歷史: ", code);
	for(int i=0;i<STATE_HISTORY;i++) printf("%d ", last_codes[i]);
	printf("\n");
    handle_state(code);                           // 呼叫 switch case 函數
}

// --------------------- main ---------------------
int main(){
    // 初始化歷史狀態為線上
    memset(last_codes,2,sizeof(last_codes));
	
	// 指定 callback
    logic_cb = logic;
	node_active = false;
	
	// 開啟裝置
	if(tcrt5000_open() != 0){
    fprintf(stderr, "TCRT5000 初始化失敗\n");
    return -1;
}

    // 啟動紅外線偵測 thread
    pthread_t tcrt_thread;
    if(pthread_create(&tcrt_thread,NULL,tcrt5000_thread_func,NULL)!=0){
        fprintf(stderr,"[ERROR] 無法啟動紅外線 thread\n");
        return -1;
    }

    // 馬達初始啟動
    set_left_motor(SPEED_INIT,1);
    set_right_motor(SPEED_INIT,1);


    // 主迴圈保持程式執行，直到 stop_flag 被設定
    while(!stop_flag){
        usleep(50000); // 0.05秒延遲，快速迴圈
    }

    // 等待紅外線 thread 結束
    pthread_join(tcrt_thread,NULL);
    stop_all_motors(); // 停車
    return 0;
}
