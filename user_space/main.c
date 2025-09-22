// 車輛入口應用程式

#include <stdio.h>      	// printf, fprintf
#include <stdlib.h>     	// exit, NULL
#include <unistd.h>     	// sleep
#include <pthread.h>    	// pthread_create, pthread_join
#include <signal.h>     	// signal, SIGINT

#include "tcrt5000.h"		// 循跡感測器 API
#include "hcsr04.h"      		// 超聲波感測器 API
#include "logic.h"      	// logic_cb callback 宣告
#include "motor_ctrl.h" 	// stop_all_motors() 與馬達控制


int main() {
    pthread_t tcrt_thread;

    // 設定停止旗標
    stop_flag = 0;

    // 開啟循跡感測器
    if(tcrt5000_open() != 0) {
        fprintf(stderr, "無法開啟 TCRT5000\n");
        return -1;
    }

    // 設定 callback
    logic_cb = logic_cb;

    // 啟動循跡執行緒
    if(pthread_create(&tcrt_thread, NULL, tcrt5000_thread_func, NULL) != 0) {
        fprintf(stderr, "無法建立循跡執行緒\n");
        return -1;
    }

    printf("循跡執行緒已啟動，按 Ctrl+C 停止...\n");

    // 主迴圈 (可加其他控制，例如避障、MQTT)
    while(1) {
        sleep(1);
    }

    // 停止
    stop_flag = 1;
    pthread_join(tcrt_thread, NULL);
    tcrt5000_close();
    stop_all_motors();

    return 0;
}



