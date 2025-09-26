// 測試走直線 5 秒

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "motor_ctrl.h"  // 引入 motor.c 的函式

int main() {
    // 訊號處理，直接使用 motor.c 的 cleanup_and_exit
    signal(SIGINT, cleanup_and_exit);
    signal(SIGTERM, cleanup_and_exit);

    // 開啟馬達設備
    if(open_motor_device() < 0) {
        printf("[ERROR] 無法開啟馬達設備\n");
        return 1;
    }

    // 設定左右馬達速度與方向 (40%, 前進)
    set_left_motor(40, 1);
    set_right_motor(40, 1);

    printf("[INFO] 馬達開始直線前進 5 秒...\n");
    sleep(5);  // 持續 5 秒

    // 停止馬達
    stop_all_motors();

    // 關閉裝置檔並退出
    cleanup_and_exit(0);

    return 0;
}
