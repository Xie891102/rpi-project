// 行車應用程式入口 main.c

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "sensors/includes.h"

// 停止旗標
int stop_flag = 0;

// 主程式
int main(){

	pthread_t tcrt5000_thread;

	// 1.開啟循跡檔案
	if(tcrt5000_open() < 0){
		fprintf(stderr, "無法開啟 TCRT5000\n");
		return -1;
	}

	// 2.建立執行緒
	if(pthread_create(&tcrt5000_thread, NULL, tcrt5000_thread_func, NULL) != 0){
		perror("tcrt5000 pthread_create failed");	// 執行緒建立失敗
		tcrt5000_close();				// 關閉 循跡偵測
		return -1;
	}

	// 3.正常可以使用，直到接收中止訊號出現
	printf("按 enter 停止...\n");
	getchar(); 			// 等待停止按鈕
					
	// 4.停止 thread
	stop_flag = 1;
	pthread_join(tcrt5000_thread, NULL);

	// 5.關閉 循跡偵測
	tcrt5000_close();

	printf("程式停止\n");
	return 0;
}





