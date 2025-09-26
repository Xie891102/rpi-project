#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

#include "../module/buzzer/include/buzzer_ioctl.h"
#include "../module/hc_sr04/include/hc_sr_ioctl.h"

#define BUZZER_GPIO 6
#define THRESHOLD_DISTANCE 50 // mm

void sonic_measure(int distances[4]);
void control_buzzer(int fd, int on);

int main() {
    int distances[4];
    int buzzer_fd;

    // 開啟蜂鳴器裝置
    buzzer_fd = open(DEVICE_FILE_NAME, O_RDWR);
    if (buzzer_fd < 0) {
        printf("Can't open buzzer device file: %s\n", DEVICE_FILE_NAME);
        exit(-1);
    }

    while (1) {
        sonic_measure(distances);
        int alert = 0;

        for (int i = 0; i < 4; i++) {
            printf("Distance%d: %d mm\n", i, distances[i]);
            if (distances[i] >= 0 && distances[i] < THRESHOLD_DISTANCE) {
                printf("sonic%d too close!\n", i);
                alert = 1;
            }
        }

        control_buzzer(buzzer_fd, alert);  // 使用副函式控制蜂鳴器
        sleep(1); // 每秒執行一次
    }

    close(buzzer_fd);
    return 0;
}

void sonic_measure(int distances[4]) {
    int trig_pin[4] = {0, 8, 10, 12};
    int echo_pin[4] = {1, 9, 11, 13};
    const char* devs[4] = {
        "/dev/ultrasonic0",
        "/dev/ultrasonic1",
        "/dev/ultrasonic2",
        "/dev/ultrasonic3"
    };

    int fds[4];
    for (int i = 0; i < 4; i++) {
        fds[i] = open(devs[i], O_RDWR);
        if (fds[i] < 0) {
            perror("open");
            distances[i] = -1;
        }
    }

    char buf[32];
    for (int i = 0; i < 4; i++) {
        distances[i] = -1;
        if (fds[i] < 0) continue;

        memset(buf, 0, sizeof(buf));
        ioctl(fds[i], HC_SR04_SET_TRIGGER, trig_pin[i]);
        ioctl(fds[i], HC_SR04_SET_ECHO, echo_pin[i]);
        int n = read(fds[i], buf, sizeof(buf));
        if (n > 0) {
            distances[i] = atoi(buf);
        }

        usleep(60000);
    }

    for (int i = 0; i < 4; i++) {
        if (fds[i] >= 0)
            close(fds[i]);
    }
}

//控制蜂鳴器
void control_buzzer(int fd, int on) {
    if (on) {
        printf("🔔 Buzzer ON\n");
        ioctl(fd, IOCTL_SET_BUZZER_ON, BUZZER_GPIO);
    } else {
        ioctl(fd, IOCTL_SET_BUZZER_OFF, BUZZER_GPIO);
    }
}