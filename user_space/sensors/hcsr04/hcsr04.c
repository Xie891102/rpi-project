// hcsr04超聲波測距
/*
#include "hcsr04.h"

hcsr04_all_data data;
hcsr04_open_all();
hcsr04_read_all(&data);
printf("%d\n", data.ultrasonic[0].distance);
hcsr04_close_all();
*/
#include "hcsr04.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h> // atoi
#include <string.h>
#include <sys/ioctl.h> 

#define HC_SR04_NUM        4
#define HC_SR04_DEV_FMT    "/dev/ultrasonic%d"
#define HC_SR04_TRIG_0     0
#define HC_SR04_ECHO_0     1
#define HC_SR04_TRIG_1     4
#define HC_SR04_ECHO_1     5
#define HC_SR04_TRIG_2     6
#define HC_SR04_ECHO_2     7
#define HC_SR04_TRIG_3     12
#define HC_SR04_ECHO_3     13
// 四顆超聲波的檔案描述符
static int fd[4] = {-1,-1,-1,-1};
static const int trig_pins[HC_SR04_NUM] = { HC_SR04_TRIG_0, HC_SR04_TRIG_1, HC_SR04_TRIG_2, HC_SR04_TRIG_3 };
static const int echo_pins[HC_SR04_NUM] = { HC_SR04_ECHO_0, HC_SR04_ECHO_1, HC_SR04_ECHO_2, HC_SR04_ECHO_3 };

typedef struct {
    int distance;
} ultrasonic_data;

hcsr04_callback distance_cb = NULL;

// ---------------- 打開四顆超聲波 ----------------
int hcsr04_open_all(void) {
    char path[32];
    for(int i=0;i<HC_SR04_NUM;i++) {
        // 建立裝置路徑，例如 /dev/ultrasonic0
        snprintf(path,sizeof(path),HC_SR04_DEV_FMT,i);
        fd[i] = open(path, O_RDWR);
        if(fd[i]<0) {
            perror("hcsr04_open_all failed");
            for(int j=0;j<i;j++){
                if(fd[j]>=0) close(fd[j]);
                fd[j] = -1;
            }
            return -1;
        }
        ioctl(fd[i], HC_SR04_SET_TRIGGER, trig_pins[i]);
        ioctl(fd[i], HC_SR04_SET_ECHO, echo_pins[i]);
    }
    return 0;
}


// ---------------- 讀取四顆超聲波距離 ----------------
int hcsr04_read_all(hcsr04_all_data *data) {
    char buf[16];
    ssize_t len;

    for(int i=0;i<HC_SR04_NUM;i++) {
        data->ultrasonic[i].distance = -1;
        if(fd[i]<0) continue;
        memset(buf, 0, sizeof(buf));
        len = read(fd[i], buf, sizeof(buf)-1);
        if(len > 0) {
            buf[len] = '\0';
            data->ultrasonic[i].distance = atoi(buf);
        }
        usleep(60000); // 測完一顆再delay
    }
    return 0;
}


// ---------------- 關閉四顆超聲波 ----------------
int hcsr04_close_all(void) {
    for(int i=0;i<HC_SR04_NUM;i++){
        if(fd[i]>=0) close(fd[i]);
        fd[i]=-1;
    }
    return 0;
}




























