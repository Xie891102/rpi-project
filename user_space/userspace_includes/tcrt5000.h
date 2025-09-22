// 標頭檔- TCRT5000紅外線循跡街口

#ifndef __INCLUDES_H__
#define __INCLUDES_H__


// 循跡紅外線執行緒宣告
void* tcrt5000_thread_func(void* arg);

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


#endif

