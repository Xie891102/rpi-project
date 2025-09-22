#include "hcsr04.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>


// ---------------- 全域變數 ----------------
// 由 main.c 定義，控制 thread 停止
extern int stop_flag;

// callback 指標，由 main.c 設定
extern hcsr04_callback distance_cb;


// ---------------- 超聲波讀取執行緒 ----------------
void* hcsr04_thread_func(void* arg) {
    hcsr04_all_data data;

    while(!stop_flag) {
        // 1. 讀取四顆距離
        if(hcsr04_read_all(&data)==0){
            // 2. 呼叫 callback，將資料傳給邏輯層
            if(distance_cb!=NULL){
                distance_cb(&data); // 回傳四顆距離
            }
        } else {
            fprintf(stderr,"hcsr04 read_all failed\n");
        }

        // 3. 控制讀取頻率，這裡每 100ms 讀一次
        usleep(100000);
    }

    return NULL;
}
