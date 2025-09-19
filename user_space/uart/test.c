// 測試 UART 傳輸用   Rpi to Pico

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "uart_thread.h"
#include "uart.h"
#include "uart_queue.h"


int count = 0;

void rx_handle(const char *data, size_t len){
	printf("Rpi 收到: %s\n", data);

	if(count < 3){
		// 每次收到pico觸發下一次對話
		uart_send("work now\n");
		count++;
	}
	
}


int main(){

	// 啟動 UART thread
	if(uart_thread_start("/dev/serial0") != 0){
		fprintf(stderr, "UART thread 啟動失敗\n");
		return -1;
	}
	
	// 註冊接收事件
	uart_set_rx_callback(rx_handle);

	// 開機先說 hello
	uart_send("hello pico\n");

	// 等待事件完成
	while(count < 3){
		usleep(100000);
	}

	// 停止 uart thread
	uart_thread_stop();
	return 0;
}











