// 測試循跡走直線，偵測0停車

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>
#include "motor_ctrl.h"
#include "tcrt5000.h"

// 執行緒停止旗標
volatile int stop_flag = 0;

// callback 指標
tcrt5000_callback logic_cb = NULL;

// 主程式用的 node_active 旗標
bool node_active = false;

// Ctrl+C 信號處理，安全停止馬達及感測器
void handle_signal(int sig) {
    printf("\n[INFO] 接收到結束訊號，停止馬達與循跡感測器...\n");
    stop_flag = 1;
    stop_all_motors();
    tcrt5000_close();
    exit(0);
}

// 主 callback 函式，執行緒讀到資料後會呼叫
void track_logic(int code) {
    if (code == 0) {          // 偵測到黑線 000
        stop_flag = 1;        // 停止執行緒
        stop_all_motors();    // 停車
        printf("[INFO] 偵測到黑線 000 - 停車\n");
    } else {
        move_forward();       	// 繼續前進

    }
}

int main() {
    pthread_t tcrt_thread;

    // 設定 Ctrl+C 信號處理
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    // 1. 開啟馬達裝置
    if (open_motor_device() < 0) {
        fprintf(stderr, "[ERROR] 無法開啟馬達裝置\n");
        return 1;
    }

    // 2. 開啟 TCRT5000 感測器
    if (tcrt5000_open() < 0) {
        fprintf(stderr, "[ERROR] 無法開啟循跡感測器\n");
        return 1;
    }

    // 3. 設定 callback 指向 track_logic
    logic_cb = track_logic;

    printf("[INFO] 循跡測試開始，速度固定 40，偵測 000 停車\n");

    // 4. 啟動循跡執行緒
    pthread_create(&tcrt_thread, NULL, tcrt5000_thread_func, NULL);

    // 5. 等待執行緒結束 (遇到 000 或 Ctrl+C)
    pthread_join(tcrt_thread, NULL);

    // 6. 確保馬達停止，關閉感測器
    stop_all_motors();
    tcrt5000_close();

    printf("[INFO] 測試完成，程式結束\n");
    return 0;
}
