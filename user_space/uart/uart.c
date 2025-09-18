// 封裝 UART device層 實現open read write release  

#include <stdio.h>
#include <fcntl.h>	// open()
#include <unistd.h>	// read(),write(),close()
#include <string.h>
#include "uart.h"


// 靜態全域描述符，追蹤 UART 裝置是否已開啟
static int uart_fd = -1;


// 開啟 uart 裝置
// 0  成功開啟
// -1 開啟失敗
int uart_open(const char *device){

	// 已經開啟則返回
	if(uart_fd >= 0) return 0;

	// 使用 open() 開啟 UART，O_RDWR可讀寫
	// O_NOCTTY 不將其設定為控制終端(防止Ctrl+C)
	uart_fd = open(device, O_RDWR | O_NOCTTY);
	if(uart_fd < 0){
		perror("uart_open failed");	// 顯示錯誤訊息
		return -1;
	}
	return 0;
}


// 關閉裝置
// 0 成功關閉  
//-1 關閉失敗
int uart_close(void){

	// 尚未開啟，直接返回
	if(uart_fd < 0) return 0;

	// 關閉 UART
	if(close(uart_fd) < 0){
		perror("uart_close failed");	// 顯示錯誤訊息
	return -1;
	}

	// 重置
	uart_fd = -1;
	return 0;
}


// 讀取UART資料
// buf: 儲存獨到的資料  len: buf的大小，最多len個bytes
int uart_read(char *buf, size_t len){
	
	// 未開啟 UART
	if(uart_fd < 0) return -1;

	// 讀取資料
	ssize_t n = read(uart_fd, buf, len);	
	if(n < 0){
		perror("uart_read failed");	// 顯示錯誤訊息
		return -1;
	}
	
	// 成功，回傳實際讀取數量
	return n;
}


// 寫入 UART
// buf: 傳送資料   len: buf長度
int uart_write(const char *buf, ssize_t len){
	
	// 未開啟
	if(uart_fd < 0) return -1;

	// 寫入
	ssize_t n = write(uart_fd, buf, len);	
	if(n < 0){
		perror("uart_write failed");	// 顯示錯誤訊息
		return -1;
	}

	// 成功，回傳實際寫入數量
	return n;
}












