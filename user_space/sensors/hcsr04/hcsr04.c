// hcsr04超聲波測距

#include "hcsr04.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h> // atoi

// 四顆超聲波的檔案描述符
static int fd[4] = {-1,-1,-1,-1};


// ---------------- 打開四顆超聲波 ----------------
int hcsr04_open_all(void) {
    char path[32];
    for(int i=0;i<4;i++) {
        // 建立裝置路徑，例如 /dev/ultrasonic0
        snprintf(path,sizeof(path),"/dev/ultrasonic%d",i);
        fd[i] = open(path,O_RDONLY);
        if(fd[i]<0) {
            perror("hcsr04_open_all failed");
            return -1;
        }
    }
    return 0;
}


// ---------------- 讀取四顆超聲波距離 ----------------
int hcsr04_read_all(hcsr04_all_data *data) {
    char buf[16];
    ssize_t len;

    for(int i=0;i<4;i++) {
        if(fd[i]<0) return -1;  // 尚未開啟
        len = read(fd[i],buf,sizeof(buf)-1);
        if(len>0){
            buf[len]='\0';
            data->ultrasonic[i].distance = atoi(buf); // 轉成整數
        } else {
            data->ultrasonic[i].distance = -1; // 讀取失敗
        }
    }
    return 0;
}


// ---------------- 關閉四顆超聲波 ----------------
int hcsr04_close_all(void) {
    for(int i=0;i<4;i++){
        if(fd[i]>=0) close(fd[i]);
        fd[i]=-1;
    }
    return 0;
}




























