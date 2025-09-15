// 路線解析 route.c 標頭檔  
#ifndef ROUTE_H
#define ROUTE_H

#include <stdio.h>
#include <stdlib.h>


// 1. 代號對照表(數字轉文字)
typedef enum {
	STRAIGHT = 1,	// 直行
	RIGHT = 2,	// 右轉
	LEFT = 3,	// 左轉
	STOP = 4	// 停止
} Action;


// 2. 路線結構體(儲存路線)
typedef struct {
	Action *steps;	// 儲存動作陣列 [右轉, 直行...]
	int length;	// 總步驟數
	int current;	// 當前為第幾步 
} Route;


// 3. 外界 對 route.c 函式呼叫入口

// 建立路線 輸入原始數字陣列
Route* create_route(int *raw_data, int len);

// 判斷是否有下一步
int has_next(Route *r);

// 取得下一步 並更新當前步驟current+1
Action next_step(Route *r);

// 將路線重新設置
void reset_route(Route *r);

// 釋放記憶體
void free_route(Route *r);

// 將 Action 轉成文字 顯示/debug使用
const char* action_to_string(Action a);

// 返回原點路線解析
Route* reverse_route(Route *r);


#endif 

