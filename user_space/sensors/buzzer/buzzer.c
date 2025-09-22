#include "buzzer.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include "buzzer_ioctl.h" 


// 蜂鳴器描述檔
static int fd = -1;  

// ---------------- 開啟蜂鳴器 ----------------
int buzzer_open(void) {
    if(fd >= 0) return 0; // 已經開啟
    fd = open(DEVICE_FILE_NAME, O_RDWR);
    if(fd < 0){
        perror("buzzer_open failed");
        return -1;
    }
    return 0;
}

// ---------------- 控制蜂鳴器 ----------------
int buzzer_write(int on) {
    if(fd < 0) {
        fprintf(stderr,"buzzer not opened\n");
        return -1;
    }

    if(on){
        printf("Buzzer ON\n");
        ioctl(fd, IOCTL_SET_BUZZER_ON, BUZZER_GPIO);
    } else {
        ioctl(fd, IOCTL_SET_BUZZER_OFF, BUZZER_GPIO);
    }

    return 0;
}

// ---------------- 關閉蜂鳴器 ----------------
int buzzer_close(void) {
    if(fd >= 0) close(fd);
    fd = -1;
    return 0;
}
