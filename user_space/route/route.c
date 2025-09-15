// 解析處理調度中心給的指示
#include "route.h"

// 1.建立路線
Route* create_route(int *raw_data, int len){

	// 檢查是否有東西
	if(len <= 0 || raw_data == NULL) return NULL;

	// 分配 Route 記憶體
	Route *r = (Route*)malloc(sizeof(Route));
	if(!r) return NULL;	// 如果沒有回傳NULL

	// 分配 steps 陣列
	r -> steps = (Action*)malloc(sizeof(Action) * len);
	if(!r->steps){		// 如果失敗 釋放資源 並回傳NULL
		free(r);
		return NULL;
	}

	// 將數字轉 Action enum
	for(int i = 0; i < len; i++){
		switch(raw_data[i]) {
			case 1: r->steps[i] = STRAIGHT; break;
			case 2: r->steps[i] = RIGHT; break;
			case 3: r->steps[i] = LEFT; break;
			case 4: r->steps[i] = STOP; break;
			default : r->steps[i] = STOP; break;	// 預設停車
		}
	}

	r->length = len;	// 總步驟數
	r->current = 0;		// 從第一步開始
	return r;
}


// 2.取得下一步
int has_next(Route *r){
	if(!r) return 0;
	return r->current < r->length;
}


// 3.判斷下一個動作
Action next_step(Route *r){
	if(!r || r->current >= r->length) return STOP;	// 超過範圍或是無效路線返回 stop
	return r->steps[r->current++];
}


// 4.重設路線
void reset_route(Route *r){
	if(!r) return;
	r->current = 0;		// 從頭開始
}


// 5.釋放記憶體(釋放 steps陣列 / Route結構體)
void free_route(Route *r){
	if(!r) return;
	if(r->steps) free(r->steps);
	free(r);
}


// 6.Action 轉文字 (debug / 顯示使用)
const char* action_to_string(Action a){
	switch(a) {
		case STRAIGHT:  return "STRAIGHT";
		case RIGHT: 	return "RIGHT";
		case LEFT: 	return "LEFT";
		case STOP:	return "STOP";
		default: 	return "UNKNOWN";
	}
}


// 7.左右邊互換
static Action reverse_action(Action a){
	switch(a) {
		case LEFT:	return RIGHT;	// 左 變成 右
		case RIGHT:	return LEFT;    // 右 變成 左
		default:	return a;	// 執行不變 
	}
}


// 8.建立回程路線
Route* reverse_route(Route *r){

	// 無效路線回傳 NULL
	if(!r || r->length <= 0) return NULL;

	// 分配新路線 Route 結構體
	Route *rev = (Route*)malloc(sizeof(Route));
	rev->length = r->length;					// 覆蓋路線長度
	rev->current = 0;						// 當前步驟-從頭開始
	rev->steps = (Action*)malloc(sizeof(Action) * r->length);	// 分配動作陣列
	
	// 倒敘路線 並左右交換	
	for(int i = 0; i < r->length; i++){
		Action original = r->steps[r->length - 1 - i];		// 從最後一步開始取
		rev->steps[i] = reverse_action(original);		// 左右互換
	}	
	return rev;	// 回傳新路線
}


