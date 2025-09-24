#ifndef __HCSR04_H__
#define __HCSR04_H__
#define HC_SR04_SET_TRIGGER  _IOW('U', 0, int)
#define HC_SR04_SET_ECHO     _IOW('U', 1, int)


// 單顆超聲波的距離資料
typedef struct {
    int distance; // 單位: cm
} hcsr04_data;


// 四顆超聲波的資料集合
typedef struct {
    hcsr04_data ultrasonic[4]; // ultrasonic[0]~[3]
} hcsr04_all_data;

// callback 型態，由邏輯層實作
typedef void(*hcsr04_callback)(hcsr04_all_data *data);


// ----------------- API -----------------
int hcsr04_open_all(void);               // 打開四顆超聲波
int hcsr04_read_all(hcsr04_all_data *d); // 讀取四顆距離
int hcsr04_close_all(void);              // 關閉四顆超聲波


// 執行緒函式，供 pthread_create 使用
void* hcsr04_thread_func(void* arg);


// callback 指標，供logic.c 設定
extern hcsr04_callback distance_cb;


#endif
