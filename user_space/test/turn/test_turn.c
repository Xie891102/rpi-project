// 測試馬達左右轉

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "motor_ctrl.h"  // 馬達控制頭檔

int main() {
    // 訊號處理 → 直接使用 motor.c 的 cleanup_and_exit
    signal(SIGINT, cleanup_and_exit);
    signal(SIGTERM, cleanup_and_exit);

    // 開啟馬達設備
    if (open_motor_device() < 0) {
        printf("[ERROR] 無法開啟馬達設備\n");
        return 1;
    }

    // 左轉 3 秒
    printf("[INFO] 馬達左轉 3 秒...\n");
    turn_left();           // 高階函式左轉
    sleep(3);
    stop_all_motors();

    // 右轉 3 秒
    printf("[INFO] 馬達右轉 3 秒...\n");
    turn_right();          // 高階函式右轉
    sleep(3);
    stop_all_motors();

    // 結束程式
    cleanup_and_exit(0);

    return 0;
}
