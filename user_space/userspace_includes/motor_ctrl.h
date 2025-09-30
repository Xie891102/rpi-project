// 馬達控制器(logic.c使用)
#ifndef MOTOR_CTRL_H
#define MOTOR_CTRL_H

// 高階控制指令 (抽象化，直接做常用的行為)
// -------------------------------------------------

// 前進 (左右馬達等速，方向為前進)
void move_forward(void);

// 後退 (左右馬達等速，方向為後退)
void move_backward(void);

// 左轉 (一般會讓右輪前進，左輪後退或停)
void turn_left(void);

// 右轉 (一般會讓左輪前進，右輪後退或停)
void turn_right(void);

// 停車 (兩邊馬達都停止)
int stop_all_motors(void);

int open_motor_device(void);


// 低階控制指令 (精細控制，適合微調 / 自訂行為)
// -------------------------------------------------

// 設定左馬達
// speed: 速度百分比 (0~100)
// direction: 方向 (1=前進, -1=後退, 0=停止)
int set_left_motor(int speed, int direction);

// 設定右馬達
// speed: 速度百分比 (0~100)
// direction: 方向 (1=前進, -1=後退, 0=停止)
int set_right_motor(int speed, int direction);

// 程式結束時清理並關閉設備
void cleanup_and_exit(int sig);

extern pthread_mutex_t motor_mutex;

// 顯示狀態 (可選, 用於 debug，例如印出速度/方向)
void display_status(void);

#endif



















