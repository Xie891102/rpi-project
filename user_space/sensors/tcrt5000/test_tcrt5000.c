// tset 測試 tcrt5000 循跡紅外線 以及紅外線的 thread
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#include "tcrt5000.h"


tcrt5000_callback logic_cb = NULL;	// callback 指標
int stop_flag = 0;
bool node_active = false;

// 印出 code(組合後)
void test_cb(int code){
	printf("TCRT5000 code = %d\n", code);
}

int main(){
	logic_cb = test_cb;	// 設定回呼函式

	if(tcrt5000_open() != 0) return -1;

	pthread_t tid;
	pthread_create(&tid, NULL, tcrt5000_thread_func, NULL);
			

	sleep(10);	// 10秒

	stop_flag = 1;

	pthread_join(tid, NULL);
	tcrt5000_close();

	return 0;
}




