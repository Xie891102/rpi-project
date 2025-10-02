//  循線大改

#include <stdio.h>      // 標準輸出入 (printf, fprintf)
#include <unistd.h>     // usleep() 用來控制延遲
#include <pthread.h>    // pthread_create() 建立紅外線感測器的執行緒
#include <stdbool.h>    // 使用 bool 型別
#include <sys/time.h>   // gettimeofday() 取得當前時間 (毫秒)
#include <string.h>     // memset, 字串相關函數

#include "tcrt5000.h"   // 紅外線感測器模組 (循跡用)
#include "motor_ctrl.h" // 馬達控制函式庫
#include "uart_thread.h"
#include "mqtt_config.h"  
 

// ============================================================================
// 參數設定 (可以依照車子的特性調整)
// ============================================================================

// 判斷「出軌」的門檻
#define DERAIL_SET_COUNT 10    // 累積 10 次判斷仍出軌，就算無法恢復
#define DERAIL_SET_TIME 2000   // 出軌後 2 秒內沒恢復，就觸發緊急停車

// 馬達速度參數
#define SPEED_INIT 40          // 馬達基礎速度 (正常直行時)
#define SPEED_MIN 40           // 馬達最低速度 (避免太低無力)
#define SPEED_MAX 70           // 馬達最高速度 (避免太快失控)
#define SPEED_MINOR 2          // 輕微修正幅度 (微左/微右)
#define SPEED_MAJOR 3           // 中度修正幅度 (太左/太右)
#define SPEED_RECOVER 5        // 出軌恢復用的強力修正
#define LEFT_INIT 10            // 起步時左輪多給 10，補償起步偏差
#define LEFT_BIAS_CORRECT 4    // 行駛過程左輪補償，因為車體偏左

// 狀態歷史紀錄 (用於分析趨勢)
#define STATE_HISTORY 10       // 保存最近 10 筆感測器狀態
#define LINEAR_CHECK_N 7       // 檢查最近 7 筆狀態
#define LINEAR_THRESHOLD 4     // 至少 3 次同方向偏移，才算有趨勢
#define DEBOUNCE_COUNT 3        // 去抖動：連續幾次相同才算有效


// ============================================================================
// 全域變數區 (紀錄狀態用)
// ============================================================================

static int left_derail_count = 0;   // 左偏累積次數 (出軌判斷)
static long left_derail_time = 0;   // 左偏開始計時
static int right_derail_count = 0;  // 右偏累積次數 (出軌判斷)
static long right_derail_time = 0;  // 右偏開始計時

volatile int stop_flag = 0;         // 停止旗標 (被設成 1 時程式就會停車)
int last_codes[STATE_HISTORY];      // 紀錄最近 10 次狀態碼
int history_idx = 0;                // 環形緩衝區索引 (指向最新狀態的位置)

bool node_active = false;           // 節點標記 (保留功能，這裡沒用到)
tcrt5000_callback logic_cb = NULL;  // 紅外線感測器的回呼函數


// ============================================================================
// 工具函數 (輔助用，不直接控制車子)
// ============================================================================

// 取得當前毫秒數 (方便算經過時間，避免用 sleep 造成阻塞)
long get_millis(void){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

// 限制馬達速度在 [SPEED_MIN, SPEED_MAX] 範圍內
int clamp_speed(int speed){
    if(speed < SPEED_MIN) return SPEED_MIN;
    if(speed > SPEED_MAX) return SPEED_MAX;
    return speed;
}

// 找出「最近一次非出軌的狀態」
// 出軌 (code=0) 不算有效，避免用錯誤的狀態做判斷
int find_last_valid(){
    for(int i = 1; i <= STATE_HISTORY; i++){
        int idx = (history_idx - i + STATE_HISTORY) % STATE_HISTORY;
        int code = last_codes[idx];
        if(code != 0) { // 只要不是出軌就當作有效
            return code;
        }
    }
    return 2;  // 如果完全找不到，預設成「直行」
}

// 判斷最近 5 次狀態裡「左偏」是否占大多數
bool is_left_shift(){
    int left_count = 0;
    for(int i = 1; i <= LINEAR_CHECK_N; i++){
        int idx = (history_idx - i + STATE_HISTORY) % STATE_HISTORY;
        int s = last_codes[idx];
        if(s == 1 || s == 3){ // 狀態 1=太左, 3=微左
            left_count++;
        }
    }
    return left_count >= LINEAR_THRESHOLD;
}

// 判斷最近 5 次狀態裡「右偏」是否占大多數
bool is_right_shift(){
    int right_count = 0;
    for(int i = 1; i <= LINEAR_CHECK_N; i++){
        int idx = (history_idx - i + STATE_HISTORY) % STATE_HISTORY;
        int s = last_codes[idx];
        if(s == 4 || s == 6){ // 狀態 4=太右, 6=微右
            right_count++;
        }
    }
    return right_count >= LINEAR_THRESHOLD;
}


// ============================================================================
// 馬達控制函數 (直接驅動左右馬達)
// ============================================================================

// 設定馬達速度
// mode=0 → 正常模式：會限制上下限
// mode=1 → 出軌恢復模式：允許速度比正常更低，但仍限制最高
void apply_motor_speed(int left_speed, int right_speed, int mode){
    // 車子本身偏左 → 左輪多給一點補償
    left_speed += LEFT_BIAS_CORRECT;

    if(mode == 0){ // 正常模式
        if(left_speed < SPEED_MIN) left_speed = SPEED_MIN;
        if(left_speed > SPEED_MAX) left_speed = SPEED_MAX;
        if(right_speed < SPEED_MIN) right_speed = SPEED_MIN;
        if(right_speed > SPEED_MAX) right_speed = SPEED_MAX;
    }
    else if(mode == 1){ // 出軌恢復模式
        // 出軌時要大幅調整，不限制最低速度，但要避免超過最大
        if(left_speed > SPEED_MAX) left_speed = SPEED_MAX;
        if(right_speed > SPEED_MAX) right_speed = SPEED_MAX;
    }

    // 呼叫底層控制函數，實際設定馬達
    set_left_motor(left_speed, 1);
    set_right_motor(right_speed, 1);
}

// 重置「出軌計數器」
// 每次狀態恢復正常，就清空紀錄，避免誤判
void reset_derail_counters(){
    left_derail_count = 0;
    left_derail_time = 0;
    right_derail_count = 0;
    right_derail_time = 0;
}


// ============================================================================
// 馬達控制邏輯 (核心：決定怎麼走)
// ============================================================================
void handle_state(int code){
    switch(code){

        // --------------------------------------------------------------
        // 狀態 0: 出軌 (完全沒偵測到線)
        // --------------------------------------------------------------
        case 0: // 出軌處理
{
    int last_valid = find_last_valid(); // 找出出軌前最後有效狀態
    long now = get_millis();

    // ------------------------------------------------------------
    // 判斷趨勢：左偏 / 右偏
    // ------------------------------------------------------------
    bool trend_left = is_left_shift();
    bool trend_right = is_right_shift();

    // ------------------------------------------------------------
    // 強力掃回模式 flag
    // ------------------------------------------------------------
    static bool recovery_mode = false;   // 是否正在強力掃回
    static long recovery_start = 0;      // 強力掃回開始時間
    static bool reversing = false;       // 是否正在倒退校正

    // ------------------------------------------------------------
    // 情境1：左偏趨勢 → 向右掃回
    // ------------------------------------------------------------
    if(trend_left && !reversing){
        recovery_mode = true;
        recovery_start = now;

        if(last_valid == 1){ // 太左偏
            apply_motor_speed(SPEED_INIT + SPEED_RECOVER,
                              SPEED_INIT - SPEED_RECOVER, 1);
        } else if(last_valid == 3){ // 微左偏
            apply_motor_speed(SPEED_INIT + SPEED_MAJOR,
                              SPEED_INIT - SPEED_MAJOR, 1);
        } else {
            apply_motor_speed(SPEED_INIT, SPEED_INIT, 0);
        }
        reset_derail_counters();
    }
    // ------------------------------------------------------------
    // 情境2：右偏趨勢 → 向左掃回
    // ------------------------------------------------------------
    else if(trend_right && !reversing){
        recovery_mode = true;
        recovery_start = now;

        if(last_valid == 4){ // 太右偏
            apply_motor_speed(SPEED_INIT - SPEED_RECOVER,
                              SPEED_INIT + SPEED_RECOVER, 1);
        } else if(last_valid == 6){ // 微右偏
            apply_motor_speed(SPEED_INIT - SPEED_MAJOR,
                              SPEED_INIT + SPEED_MAJOR, 1);
        } else {
            apply_motor_speed(SPEED_INIT, SPEED_INIT, 0);
        }
        reset_derail_counters();
    }
    // ------------------------------------------------------------
    // 情境3：無明確趨勢 → 嘗試累積掃回或倒退
    // ------------------------------------------------------------
    else {
        // 判斷左偏累積
        if(last_valid == 1 || last_valid == 3){
            if(left_derail_count == 0) left_derail_time = now;
            left_derail_count++;
            apply_motor_speed(SPEED_INIT + SPEED_RECOVER,
                              SPEED_INIT - SPEED_RECOVER, 1);
            recovery_mode = true;
            recovery_start = now;

            long elapsed = now - left_derail_time;
            // 出軌超時 → 倒退回最後有效位置
            if(left_derail_count >= DERAIL_SET_COUNT && elapsed >= DERAIL_SET_TIME){
                stop_all_motors();
                reversing = true; // 開始倒退校正
                printf("[倒退校正] 左偏出軌無法恢復，倒退...\n");
            }
        }
        // 判斷右偏累積
        else if(last_valid == 4 || last_valid == 6){
            if(right_derail_count == 0) right_derail_time = now;
            right_derail_count++;
            apply_motor_speed(SPEED_INIT - SPEED_RECOVER,
                              SPEED_INIT + SPEED_RECOVER, 1);
            recovery_mode = true;
            recovery_start = now;

            long elapsed = now - right_derail_time;
            if(right_derail_count >= DERAIL_SET_COUNT && elapsed >= DERAIL_SET_TIME){
                stop_all_motors();
                reversing = true;
                printf("[倒退校正] 右偏出軌無法恢復，倒退...\n");
            }
        }
    }

    // ------------------------------------------------------------
    // 掃回後檢測是否重新掃到黑線 → 收斂速差，進入正常模式
    // ------------------------------------------------------------
    if(recovery_mode && last_valid == 2){ // 偵測到線
        recovery_mode = false;
        // 將左右速差降到平滑值，避免過度蛇行
        apply_motor_speed(SPEED_INIT + SPEED_MINOR,
                          SPEED_INIT - SPEED_MINOR, 0);
        reset_derail_counters();
        printf("[恢復穩定] 掃到線，速差收斂，恢復正常循跡\n");
    }

    // ------------------------------------------------------------
    // 倒退校正邏輯
    // ------------------------------------------------------------
    if(reversing){
        // 低速倒退
        set_left_motor(clamp_speed(SPEED_INIT - 20), 1);  // 左右同速低速倒退
        set_right_motor(clamp_speed(SPEED_INIT - 20), 1);

        // 如果倒退期間偵測到黑線 → 停止倒退，恢復正常
        if(last_valid == 2){
            reversing = false;
            reset_derail_counters();
            apply_motor_speed(SPEED_INIT, SPEED_INIT, 0);
            printf("[倒退完成] 回到線上，恢復循跡\n");
        }
    }
}
break;


        // --------------------------------------------------------------
        // 狀態 1: 太左偏 → 右轉大一點
        // --------------------------------------------------------------
        case 1:
            apply_motor_speed(SPEED_INIT + SPEED_MAJOR,
                              SPEED_INIT - SPEED_MINOR, 0);
            reset_derail_counters();
            break;

        // --------------------------------------------------------------
        // 狀態 2: 正常在線 → 直行
        // --------------------------------------------------------------
        case 2:
            apply_motor_speed(SPEED_INIT, SPEED_INIT, 0);
            reset_derail_counters();
            break;

        // --------------------------------------------------------------
        // 狀態 3: 微左偏 → 輕微右轉
        // --------------------------------------------------------------
        case 3:
            apply_motor_speed(SPEED_INIT + SPEED_MINOR,
                              SPEED_INIT - SPEED_MINOR, 0);
            reset_derail_counters();
            break;

        // --------------------------------------------------------------
        // 狀態 4: 太右偏 → 左轉大一點
        // --------------------------------------------------------------
        case 4:
            apply_motor_speed(SPEED_INIT - SPEED_MINOR,
                              SPEED_INIT + SPEED_MAJOR, 0);
            reset_derail_counters();
            break;

        // --------------------------------------------------------------
        // 狀態 5: 終點 → 停止
        // --------------------------------------------------------------
        case 5:
            stop_all_motors();
	    mqtt_publish("statusMSG/car", "arrived");	// 通報調度中心
		uart_send("R");
            printf("[STOP] 到達終點\n");
            stop_flag = 1;
            break;

        // --------------------------------------------------------------
        // 狀態 6: 微右偏 → 輕微左轉
        // --------------------------------------------------------------
        case 6:
            apply_motor_speed(SPEED_INIT - SPEED_MINOR,
                              SPEED_INIT + SPEED_MINOR, 0);
            reset_derail_counters();
            break;

        // --------------------------------------------------------------
        // 狀態 7: 節點 (交叉點) → 直行
        // --------------------------------------------------------------
        case 7:
	{
    	static bool turning = false;  // 確保轉彎過程只執行一次

    	if(!turning){
        		turning = true;

        		printf("[NODE] 直角轉彎節點觸發\n");

        		// 1️.關閉紅外線 callback，避免掃回干擾
        		node_active = true;

			uart_send("L");
       	 	// 2️.停車並等待 1 秒讓車子穩定
        		stop_all_motors();
       	 	usleep(1000000);  // 1 秒

       	 	// 3️.執行轉彎函式
        		turn_left();      		 // 或 turn_right()，依你的路線設計
        		usleep(1300000);  		// 1.3秒

       	 	// 4️.停車確認角度
        		stop_all_motors();
       		usleep(500000);    		// 0.5秒短暫停車
		mqtt_publish("statusMSG/car", "node");	// 通報調度中心


        		// 5️.開啟紅外線 callback，恢復循跡
        		node_active = false;
			
			uart_send("l");


        		// 6️馬達直行
        		apply_motor_speed(SPEED_INIT, SPEED_INIT, 0);

        		// 7.重置 turning 狀態，方便下次節點使用
        		turning = false;
    	}

    	reset_derail_counters();
}
break;


        // --------------------------------------------------------------
        // 其他未知狀態 → 預設直行
        // --------------------------------------------------------------
        default:
            apply_motor_speed(SPEED_INIT, SPEED_INIT, 0);
            printf("[WARN] 未知狀態 %d\n", code);
            reset_derail_counters();
            break;
    }
}


// ============================================================================
// 回呼函數 (感測器呼叫這裡，把狀態交給邏輯處理)
// ============================================================================
void logic(int code){
    static int last_code = -1;   // 上一次的狀態
    static int repeat_count = 0; // 連續次數

    if(code == last_code){
        repeat_count++;
    } else {
        repeat_count = 1;
        last_code = code;
    }

    // 連續出現 N 次才採用
    if(repeat_count >= DEBOUNCE_COUNT){
        last_codes[history_idx] = code;		// 把當前狀態存進歷史紀錄
        history_idx = (history_idx + 1) % STATE_HISTORY;	// 更新索引 (環形陣列)
        handle_state(code);			// 執行對應動作
        repeat_count = 0; 			// 重置，避免卡住
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
// 主程式入口
// ============================================================================
int main(){
    printf("循跡車控制程式 - 出軌恢復版\n");

    // 初始化狀態緩衝區 (一開始預設為「直行」)
    for(int i = 0; i < STATE_HISTORY; i++){
        last_codes[i] = 2;
    }

    // 設定回呼函數
    logic_cb = logic;
    node_active = false;

    // 開啟馬達裝置
    if (open_motor_device() < 0) {
        fprintf(stderr, "[ERROR] 無法開啟馬達裝置\n");
        return 1;
    }

    // 初始化紅外線感測器
    if(tcrt5000_open() != 0){
        fprintf(stderr, "[ERROR] TCRT5000 初始化失敗\n");
        return -1;
    }
    printf("[OK] TCRT5000 初始化成功\n");

    // 啟動感測器執行緒 (50Hz = 每 20ms 取樣一次)
    pthread_t tcrt_thread;
    if(pthread_create(&tcrt_thread, NULL, tcrt5000_thread_func, NULL) != 0){
        fprintf(stderr, "[ERROR] 無法啟動執行緒\n");
        tcrt5000_close();
        return -1;
    }
    printf("[OK] 紅外線執行緒啟動 (50Hz)\n");

    // 啟動馬達 (左輪加速，避免車子偏左)
    set_left_motor(clamp_speed(SPEED_INIT + LEFT_INIT), 1);
    set_right_motor(SPEED_INIT, 1);    
    printf("[OK] 馬達啟動, 開始循跡\n");

// ---------------- UART 啟動 ----------------
    if(uart_thread_start("/dev/ttyS0") != 0){
        fprintf(stderr, "[ERROR] UART 啟動失敗\n");
        return -1;
    }
    printf("[OK] UART 執行緒啟動\n");
uart_send("o");
uart_send("i");
uart_send("r");
uart_send("l");


// 初始化 MQTT
    if(mqtt_init() != 0){
        fprintf(stderr, "[ERROR] MQTT 初始化失敗\n");
        stop_flag = 1;
    } else {
        // 訂閱控制訊息，例如 "statusMSG/callingCar"
        mqtt_subscribe("statusMSG/callingCar", mqtt_callback_func);
        printf("[OK] MQTT 已連線並訂閱控制 Topic\n");
    }


    // 主迴圈 (直到 stop_flag=1 才結束)
    while(!stop_flag){
    if(!node_active){
        stop_all_motors();
    }
    usleep(50000);
}

	uart_send("r");
	uart_send("l");

    // 程式結束 → 清理資源
    printf("循跡車控制程式正常結束\n");
    pthread_join(tcrt_thread, NULL); // 等待感測器執行緒結束
    stop_all_motors();               // 停止馬達
    tcrt5000_close();                // 關閉感測器
    mqtt_close();		// 關閉 MQTT

    printf("[OK] 清理完成\n");
    return 0;
}
