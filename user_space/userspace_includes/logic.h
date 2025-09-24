#ifndef __LOGIC_H__
#define __LOGIC_H__

#include <stdbool.h>
#include "route.h"
#include "hcsr04.h"

// 節點鎖定旗標 (外部可用 extern 引用)
extern bool node_active;

// ----------------- 超聲波避障 -----------------

// 緊急停止，停止馬達、蜂鳴器、紅燈，並通知 MQTT 調度中心
void emergency_stop(void);

// 解除緊急狀態，關閉蜂鳴器與紅燈，並通知 MQTT 調度中心
void emergency_clear(void);

// 超聲波 callback，hcsr04_thread_func 讀到距離時呼叫
void hcsr04_callback(hcsr04_all_data *data);


// ----------------- 節點處理 -----------------

// 讀取下一節點，並控制馬達與燈號
void handle_node(Route *route);


// ----------------- 循跡邏輯 -----------------

// 根據紅外線編碼判斷行駛邏輯
void logic(int code);

#endif
