/**
 * ============================================================================
 * 循跡車控制程式 - 循跡直線 + 出軌恢復版 + mqtt版
 * ============================================================================
 * 
 * 【出軌恢復策略優化】
 * 
 * 原始問題:
 *   - 出軌後立即停車,沒有嘗試恢復
 *   - 短暫離線就觸發停止,太過敏感
 * 
 * 新的設計:
 *   1. 偵測出軌 → 立即根據歷史趨勢「掃回」
 *   2. 左偏出軌 → 大幅度右轉掃回 (左輪加速、右輪減速)
 *   3. 右偏出軌 → 大幅度左轉掃回 (右輪加速、左輪減速)
 *   4. 只有在「掃回失敗(持續出軌太久)」才停車
 * 
 * 這樣可以:
 *   - 提高容錯率,允許短暫離線
 *   - 主動尋找黑線,而不是被動等待
 *   - 更符合實際循跡車的運作邏輯
 * 
 * ============================================================================
 */

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/time.h>
#include <string.h>

#include "tcrt5000.h"
#include "motor_ctrl.h"
#include "mqtt_config.h"   // 你寫好的 MQTT API

// ============================================================================
// 設定參數區
// ============================================================================
#define DERAIL_SET_COUNT 10       // 改為 10 次 (給更多恢復機會)
#define DERAIL_SET_TIME 200     // 改為 2000ms (2秒,更寬容)

#define SPEED_INIT 40            // 馬達基礎速度
#define SPEED_MIN 40             // 馬達最低速度
#define SPEED_MAX 70            // 馬達最高速度
#define SPEED_MINOR 5            // 微調修正幅度
#define SPEED_MAJOR 10           // 大幅修正幅度
#define SPEED_RECOVER 15         // 出軌恢復專用 - 更大幅度的掃回速度

#define STATE_HISTORY 10         // 紀錄最近 10 次狀態
#define LINEAR_CHECK_N 5         // 檢查最近 5 次狀態判斷趨勢
#define LINEAR_THRESHOLD 3       // 至少 3 次偏移才算有趨勢

// ============================================================================
// 全域變數區
// ============================================================================
static int left_derail_count = 0;
static long left_derail_time = 0;
static int right_derail_count = 0;
static long right_derail_time = 0;

volatile int stop_flag = 0;
int last_codes[STATE_HISTORY];
int history_idx = 0;

bool node_active = false;
tcrt5000_callback logic_cb = NULL;

// ============================================================================
// 工具函數區
// ============================================================================
long get_millis(void){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int clamp_speed(int speed){
    if(speed < SPEED_MIN) return SPEED_MIN;
    if(speed > SPEED_MAX) return SPEED_MAX;
    return speed;
}

int find_last_valid(){
    for(int i = 1; i <= STATE_HISTORY; i++){
        int idx = (history_idx - i + STATE_HISTORY) % STATE_HISTORY;
        int code = last_codes[idx];
        if(code != 0) {
            return code;
        }
    }
    return 2;  // 預設正常
}

bool is_left_shift(){
    int left_count = 0;
    for(int i = 1; i <= LINEAR_CHECK_N; i++){
        int idx = (history_idx - i + STATE_HISTORY) % STATE_HISTORY;
        int s = last_codes[idx];
        if(s == 1 || s == 3){
            left_count++;
        }
    }
    return left_count >= LINEAR_THRESHOLD;
}

bool is_right_shift(){
    int right_count = 0;
    for(int i = 1; i <= LINEAR_CHECK_N; i++){
        int idx = (history_idx - i + STATE_HISTORY) % STATE_HISTORY;
        int s = last_codes[idx];
        if(s == 4 || s == 6){
            right_count++;
        }
    }
    return right_count >= LINEAR_THRESHOLD;
}

void apply_motor_speed(int left_speed, int right_speed){
    left_speed = clamp_speed(left_speed);
    right_speed = clamp_speed(right_speed);
    set_left_motor(left_speed, 1);
    set_right_motor(right_speed, 1);
}

void reset_derail_counters(){
    left_derail_count = 0;
    left_derail_time = 0;
    right_derail_count = 0;
    right_derail_time = 0;
}

// ============================================================================
// 馬達控制邏輯函數
// ============================================================================
void handle_state(int code){
    switch(code){
        // ====================================================================
        // 狀態 0: 出軌處理 - 優化版本
        // ====================================================================
        case 0:
        {
            int last_valid = find_last_valid();  // 找出軌前的最後有效狀態
            
            // ================================================================
            // 情境1: 有明確「左偏」趨勢 → 應該往右掃回
            // ================================================================
            if(is_left_shift()){
                printf("[出軌恢復] 檢測到左偏趨勢 → 執行右掃修正\n");
                
                // 根據出軌前的狀態決定修正力道
                if(last_valid == 1){  // 出軌前是「太左偏」
                    // 需要大幅度右轉才能回到線上
                    // 左輪加速(推動車身右轉) + 右輪大幅減速(減少阻力)
                    apply_motor_speed(
                        SPEED_INIT + SPEED_RECOVER,  // 左輪: 40 + 15 = 55
                        SPEED_INIT - SPEED_RECOVER   // 右輪: 40 - 15 = 25
                    );
                    printf("  └ 使用「大幅右轉」模式 (太左偏恢復)\n");
                }
                else if(last_valid == 3){  // 出軌前是「微左偏」
                    // 中等力道右轉即可
                    apply_motor_speed(
                        SPEED_INIT + SPEED_MAJOR,    // 左輪: 40 + 10 = 50
                        SPEED_INIT - SPEED_MAJOR     // 右輪: 40 - 10 = 30
                    );
                    printf("  └ 使用「中度右轉」模式 (微左偏恢復)\n");
                }
                else{  // 其他狀態 → 保守直行
                    apply_motor_speed(SPEED_INIT, SPEED_INIT);
                    printf("  └ 使用「直行」模式 (未知狀態)\n");
                }
                
                // 重置出軌計數器 (因為我們正在主動恢復)
                reset_derail_counters();
            }
            
            // ================================================================
            // 情境2: 有明確「右偏」趨勢 → 應該往左掃回
            // ================================================================
            else if(is_right_shift()){
                printf("[出軌恢復] 檢測到右偏趨勢 → 執行左掃修正\n");
                
                // 根據出軌前的狀態決定修正力道
                if(last_valid == 4){  // 出軌前是「太右偏」
                    // 需要大幅度左轉才能回到線上
                    // 右輪加速(推動車身左轉) + 左輪大幅減速(減少阻力)
                    apply_motor_speed(
                        SPEED_INIT - SPEED_RECOVER,  // 左輪: 40 - 15 = 25
                        SPEED_INIT + SPEED_RECOVER   // 右輪: 40 + 15 = 55
                    );
                    printf("  └ 使用「大幅左轉」模式 (太右偏恢復)\n");
                }
                else if(last_valid == 6){  // 出軌前是「微右偏」
                    // 中等力道左轉即可
                    apply_motor_speed(
                        SPEED_INIT - SPEED_MAJOR,    // 左輪: 40 - 10 = 30
                        SPEED_INIT + SPEED_MAJOR     // 右輪: 40 + 10 = 50
                    );
                    printf("  └ 使用「中度左轉」模式 (微右偏恢復)\n");
                }
                else{  // 其他狀態 → 保守直行
                    apply_motor_speed(SPEED_INIT, SPEED_INIT);
                    printf("  └ 使用「直行」模式 (未知狀態)\n");
                }
                
                // 重置出軌計數器 (因為我們正在主動恢復)
                reset_derail_counters();
            }
            
            // ================================================================
            // 情境3: 無明確趨勢 → 累積計數,達標則停車
            // ================================================================
            else {
                printf("[出軌警告] 無明確偏移趨勢,開始累積計數\n");
                
                // 根據最後狀態判斷是左偏還是右偏導致
                if(last_valid == 1 || last_valid == 3){  
                    // ---------------------------------------------------
                    // 左偏導致的出軌累積
                    // ---------------------------------------------------
                    if(left_derail_count == 0){
                        left_derail_time = get_millis();  // 記錄起始時間
                        printf("  └ 開始左偏出軌計時\n");
                    }
                    left_derail_count++;
                    
                    long elapsed = get_millis() - left_derail_time;
                    printf("[左偏累積] %d/%d 次 | 持續 %ld/%d ms\n",
                           left_derail_count, DERAIL_SET_COUNT,
                           elapsed, DERAIL_SET_TIME);
                    
                    // 仍嘗試往右掃回 (但不重置計數器)
                    apply_motor_speed(
                        SPEED_INIT + SPEED_RECOVER,
                        SPEED_INIT - SPEED_RECOVER
                    );
                    
                    // 達到停車條件 (次數 AND 時間)
                    if(left_derail_count >= DERAIL_SET_COUNT &&
                       elapsed >= DERAIL_SET_TIME){
                        stop_all_motors();
						mqtt_publish("statusMSG/car", "carOut");
                        // stop_flag = 1;
                    }
                }
                else if(last_valid == 4 || last_valid == 6){
                    // ---------------------------------------------------
                    // 右偏導致的出軌累積
                    // ---------------------------------------------------
                    if(right_derail_count == 0){
                        right_derail_time = get_millis();  // 記錄起始時間
                        printf("  └ 開始右偏出軌計時\n");
                    }
                    right_derail_count++;
                    
                    long elapsed = get_millis() - right_derail_time;
                    printf("[右偏累積] %d/%d 次 | 持續 %ld/%d ms\n",
                           right_derail_count, DERAIL_SET_COUNT,
                           elapsed, DERAIL_SET_TIME);
                    
                    // 仍嘗試往左掃回 (但不重置計數器)
                    apply_motor_speed(
                        SPEED_INIT - SPEED_RECOVER,
                        SPEED_INIT + SPEED_RECOVER
                    );
                    
                    // 達到停車條件 (次數 AND 時間)
                    if(right_derail_count >= DERAIL_SET_COUNT &&
                       elapsed >= DERAIL_SET_TIME){
                        stop_all_motors();
						mqtt_publish("statusMSG/car", "carOut");
						// stop_flag = 1;
                    }
                }
            }
        }
        break;

        // ====================================================================
        // 狀態 1: 太左偏 → 大幅右轉
        // ====================================================================
        case 1:
            apply_motor_speed(SPEED_INIT + SPEED_MAJOR, SPEED_INIT - SPEED_MINOR);
            reset_derail_counters();
            break;

        // ====================================================================
        // 狀態 2: 正常線上 → 直行
        // ====================================================================
        case 2:
            apply_motor_speed(SPEED_INIT, SPEED_INIT);
            reset_derail_counters();
            break;

        // ====================================================================
        // 狀態 3: 微左偏 → 輕微右轉
        // ====================================================================
        case 3:
            apply_motor_speed(SPEED_INIT + SPEED_MINOR, SPEED_INIT - SPEED_MINOR);
            reset_derail_counters();
            break;

        // ====================================================================
        // 狀態 4: 太右偏 → 大幅左轉
        // ====================================================================
        case 4:
            apply_motor_speed(SPEED_INIT - SPEED_MINOR, SPEED_INIT + SPEED_MAJOR);
            reset_derail_counters();
            break;

        // ====================================================================
        // 狀態 5: 終點 → 停止
        // ====================================================================
        case 5:
            stop_all_motors();
            printf("[STOP] 到達終點\n");
			mqtt_publish("statusMSG/car", "readyResGoods");
            stop_flag = 1;
            break;

        // ====================================================================
        // 狀態 6: 微右偏 → 輕微左轉
        // ====================================================================
        case 6:
            apply_motor_speed(SPEED_INIT - SPEED_MINOR, SPEED_INIT + SPEED_MINOR);
            reset_derail_counters();
            break;

        // ====================================================================
        // 狀態 7: 節點 → 直行
        // ====================================================================
        case 7:
            apply_motor_speed(SPEED_INIT, SPEED_INIT);
            reset_derail_counters();
            break;

        // ====================================================================
        // 預設: 未知狀態 → 直行
        // ====================================================================
        default:
            apply_motor_speed(SPEED_INIT, SPEED_INIT);
            printf("[WARN] 未知狀態 %d\n", code);
            reset_derail_counters();
            break;
    }
}


// MQTT callback：接收指令 (例如 "receivegoods")
void mqtt_callback_func(const char *topic, const char *msg){
    if(strcmp(msg, "receivegoods") == 0){
        node_active = true;   // 啟動循跡
        printf("[MQTT] 收到啟動指令 → 啟動循跡\n");
    }
}

// ============================================================================
// 主回調函數
// ============================================================================
void logic(int code){
    last_codes[history_idx] = code;
    history_idx = (history_idx + 1) % STATE_HISTORY;
    
    #ifdef DEBUG_MODE
    printf("[LOGIC] 狀態:%d | 歷史:", code);
    for(int i = 0; i < STATE_HISTORY; i++){
        int idx = (history_idx - 1 - i + STATE_HISTORY) % STATE_HISTORY;
        printf("%d ", last_codes[idx]);
    }
    printf("\n");
    #endif
    
    handle_state(code);
}

// ============================================================================
// 主程式
// ============================================================================
int main(){
	
	printf("╔════════════════════════════════════════════╗\n");
    printf("║     循跡車控制程式 - 直線循跡+MQTT	 	     ║\n");
    printf("╚════════════════════════════════════════════╝\n\n");
	
    // 初始化狀態歷史緩衝
    for(int i = 0; i < STATE_HISTORY; i++){
        last_codes[i] = 2;
    }
    logic_cb = logic;     // 把邏輯回呼綁定
    node_active = false;  // 初始不動，等 MQTT 指令

    // 初始化 TCRT5000 紅外線模組
    if(tcrt5000_open() != 0){
        fprintf(stderr, "[ERROR] TCRT5000 初始化失敗\n");
        return -1;
    }
    printf("[OK] TCRT5000 初始化成功\n");

    // 啟動紅外線執行緒 (50Hz)
    pthread_t tcrt_thread;
    if(pthread_create(&tcrt_thread, NULL, tcrt5000_thread_func, NULL) != 0){
        fprintf(stderr, "[ERROR] 無法啟動紅外線執行緒\n");
        tcrt5000_close();
        return -1;
    }
    printf("[OK] 紅外線執行緒啟動 (50Hz)\n");

    // 初始化 MQTT
    if(mqtt_init() != 0){
        fprintf(stderr, "[ERROR] MQTT 初始化失敗\n");
        stop_flag = 1;
    } else {
        // 訂閱控制訊息，例如 "statusMSG/callingCar"
        mqtt_subscribe("statusMSG/callingCar", mqtt_callback_func);
        printf("[OK] MQTT 已連線並訂閱控制 Topic\n");
    }

    // 主迴圈
    while(!stop_flag){
        if(node_active){
            // 一旦啟動，馬達保持運轉 (循跡邏輯在 logic_cb → handle_state)
            set_left_motor(SPEED_INIT, 1);
            set_right_motor(SPEED_INIT, 1);
        } else {
            // 尚未啟動時，保持停止狀態
            stop_all_motors();
        }
        usleep(50000);
    }

    // 等待紅外線執行緒結束
    pthread_join(tcrt_thread, NULL);

    // 停止馬達 & 關閉紅外線
    stop_all_motors();
    tcrt5000_close();

    // 關閉 MQTT
    mqtt_close();

    printf("[OK] 清理完成，程式結束\n");
    return 0;
}