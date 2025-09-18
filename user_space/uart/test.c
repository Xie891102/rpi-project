// 測試 UART 傳輸用   Rpi to Pico

#define MOCK_UART

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "uart_thread.h"
#include "uart.h"
#include "uart_queue.h"


// 模擬 uart_read: 每次呼叫回傳固定字串，最多三次
int mock_read(char *buf, size_t len){
	static int cnt = 0;
	if(cnt < 3){
		strncpy(buf, "mock_data\n", len-1);
		buf[len-1] = '\0';
		cnt++;
		return strlen(buf);
	}
	return 0;	// 沒資料
}


int main(){
	
	char buf[UART_DATA_MAX];

	// 啟動 UART thread
	if(uart_thread_start("/dev/serial0") != 0){
		fprintf(stderr, "UART thread 啟動失敗\n");
		return -1;
	}

	// 發送測試資料
	uart_send("left_led_on\n");
	uart_send("right_led_on\n");
	uart_send("motor_start\n");

	// 模擬等待接收資料
	for(int i = 0; i < 10; i++){
		int n = uart_receive(buf, sizeof(buf));
		if(n > 0){
			printf("接收到: %s\n", buf);
		}
		usleep(500000);
	}

	// 停止 uart thread
	uart_thread_stop();
	return 0;
}











