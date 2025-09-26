// motor_tcrt_turn

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/time.h>
#include "motor_ctrl.h"
#include "tcrt5000.h"

// 馬達速度設定
#define SPEED_INIT   40     // 基本速度百分比
#define SPEED_CORRECT 3     // 修正幅度

// 停止旗標
volatile int stop_flag = 0;

// 節點狀態
bool node_active = false;

// 出軌緩衝計數
static int off_track_count = 0;

// 轉彎控制
static bool turning = false;
static struct timeval turn_start;
static int turn_duration_ms = 600;  // 轉彎時間

// callback 指標
tcrt5000_callback logic_cb = NULL;

// 取得當前毫秒
long get_millis() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
}

// callback 函式
void node_callback(int code) {
    // 先處理轉彎非阻塞
    if(turning){
        long now = get_millis();
        if(now - (turn_start.tv_sec*1000 + turn_start.tv_usec/1000) >= turn_duration_ms){
            turning = false;
            stop_all_motors();
            move_forward();
            printf("[ACTION] 轉彎完成，繼續前進\n");
        } else {
            // 轉彎期間不做其他修正
            return;
        }
    }

    switch(code){

        case 0: // 出軌
            off_track_count++;
            if(off_track_count >= 3){ // 連續3次才停車
                stop_all_motors();
                printf("[LOGIC] 出軌！停車並啟動尋線模式\n");
                off_track_count = 0;
            }
            break;

        default:
            off_track_count = 0; // 回到線上就重置
            break;

        case 2: // 正常在線上
            move_forward();
            break;

        case 1: // 太左偏
            set_left_motor(SPEED_INIT - SPEED_CORRECT, 1);
            set_right_motor(SPEED_INIT + SPEED_CORRECT, 1);
            break;

        case 3: // 微左偏
            set_left_motor(SPEED_INIT, 1);
            set_right_motor(SPEED_INIT + SPEED_CORRECT, 1);
            break;

        case 4: // 太右偏
            set_left_motor(SPEED_INIT + SPEED_CORRECT, 1);
            set_right_motor(SPEED_INIT - SPEED_CORRECT, 1);
            break;

        case 6: // 微右偏
            set_left_motor(SPEED_INIT + SPEED_CORRECT, 1);
            set_right_motor(SPEED_INIT, 1);
            break;

        case 5: // 終點
            stop_all_motors();
            stop_flag = 1;
            printf("[INFO] 到達終點\n");
            break;

        case 7: // 節點
            if(!node_active){
                node_active = true;
                turning = true;
                gettimeofday(&turn_start, NULL);
                stop_all_motors();
                turn_left(); // 開始轉彎
                printf("[ACTION] 遇到節點，左轉中...\n");
                node_active = false; // 轉彎後再回到 false
            }
            break;
    }
}


// main
// --------------------
int main() {
    pthread_t tcrt_thread;

    // 設定 callback
    logic_cb = node_callback;

    // 1. 開啟馬達設備
    if (open_motor_device() < 0) {
        printf("[ERROR] 無法開啟馬達\n");
        return 1;
    }

    // 2. 開啟 TCRT5000 循跡裝置
    if (tcrt5000_open() < 0) {
        printf("[ERROR] 無法開啟循跡裝置\n");
        return 1;
    }

    // 3. 開啟循跡執行緒
    pthread_create(&tcrt_thread, NULL, tcrt5000_thread_func, NULL);

    // 4. 設定初始速度直線前進
    move_forward();

    // 5. 主迴圈只等待 stop_flag
    while (!stop_flag) {
        usleep(50000); // 50ms 循環間隔
    }

    // 6. 清理
    stop_all_motors();
    pthread_join(tcrt_thread, NULL);
    tcrt5000_close();

    printf("[INFO] 測試完成\n");
    return 0;
}


