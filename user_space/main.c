// 行車應用程式入口 main.c

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "sensors/includes.h"


	
// 1.全域終止旗標
int stop_flag = 0; 	// 0繼續執行  1結束程式

// 2.每個感測器需要一組 互斥鎖mutex / 條件變數cond
	
// ---------- TCRT5000 循跡紅外線 ----------
pthread_mutex_t tcrt_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t tcrt_cond = PTHREAD_COND_INITIALIZER;
pthread_t tcrt_thread;	// tcrt5000 的 thread 代號
	
// ---------- SRF05 超音波測距 ----------
pthread_mutex_t srf05_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t srf05_cond = PTHREAD_COND_INITIALIZER;
pthread_t srf05_thread;	// srf05 的 thread 代號

// 3.宣告外部函式
void* tcrt5000_thread_func(void* arg);	// tcrt5000 thread主函式
void* srf05_thread_func(void* arg);	// srf05 thread 主函式
void logic_controller();	// 大腦邏輯迴圈

// 4.主程式
int main(int argc, char* argv[]){
	printf("[MAIN] 初始化開始\n");

	// a.啟動感測器
	// pthread_create( thread變數, thread屬性, thread函式, 傳入參數  )
	if(pthread_create(&tcrt_thread, NULL, tcrt5000_thread_func, NULL) != 0){
		perror("TCRT5000 thread 建立失敗");
		exit(1);
	}	

	if(pthread_create(&srf05_thread, NULL, srf05_thread_func, NULL) != 0){
		perror("SRF05 thread 建立失敗");
		exit(1);
	}

	printf("[MAIN] 感測器 threads 已啟動\n");
	
	// b.將控制權交給 車體大腦
	logic_controller();

	printf("[MAIN] 準備結束 通知所有threads結束...\n");

	// c.設定stop_flag = 1, 將threads退出
	pthread_mutex_lock(&tcrt_mutex);
	stop_flag = 1;
	pthread_cond_signal(&tcrt_cond);	// 叫醒
	pthread_mutex_unlock(&tcrt_mutex);

	pthread_mutex_lock(&srf05_mutex);
	pthread_cond_signal(&srf05_cond);	// 叫醒
	pthread_mutex_unlock(&srf05_mutex);

	// d.等待所有thread結束
	pthread_join(tcrt_thread, NULL);
	pthread_join(srf05_thread, NULL);

	printf("[MAIN] 已回收所有threads，程式結束\n");
	return 0;
}



