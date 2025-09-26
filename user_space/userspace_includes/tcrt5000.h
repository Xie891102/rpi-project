// 標頭檔- TCRT5000紅外線循跡街口

#ifndef __TCRT5000_H__
#define __TCRT5000_H__

#include <stdbool.h>  // bool
#include <pthread.h>

// 循跡紅外線執行緒宣告
void* tcrt5000_thread_func(void* arg);

// callback 型態，必須先宣告
typedef void(*tcrt5000_callback)(int code);

// 全域變數宣告 (在 main.c 定義)
extern volatile int stop_flag;
extern bool node_active;
extern tcrt5000_callback logic_cb;

// ----------- 結構體 --------------
// 存放回傳值 對應到 左中右
typedef struct {
	int left;	// 左邊感測器 0/1
	int middle;	// 中間感測器 0/1
	int right;	// 右邊感測器 0/1
} tcrt5000_data;


// callback 型態
typedef void(*tcrt5000_callback)(int code);

// ------------ API 介面 -------------

// 開啟裝置  回傳=> 0成功 -1失敗
int tcrt5000_open(void);

// 讀取一次資料  回傳=> 0成功  -1失敗
int tcrt5000_read(tcrt5000_data *data);

// 關閉裝置  回傳=> 0成功 -1失敗
int tcrt5000_close(void);

// 執行緒的讀取資料 
void* tcrt5000_thread_func(void* arg);


#endif