//use raspi 4B UART3
//GPIO4 = TXD3
//GPIO5 = RXD3
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>

#define UART_PORT "/dev/ttyAMA3"//預設UART請改為/dev/ttyAMA0
#define BAUD B9600

int main() {
    int fd = open(UART_PORT, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("open uart");
        return 1;
    }

    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, BAUD);
    cfsetospeed(&options, BAUD);
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CRTSCTS;
    tcsetattr(fd, TCSANOW, &options);

    // 傳送訊息到 ESP32(傳資料看這裡)
    const char *msg = "Hello ESP32!\n";
    write(fd, msg, strlen(msg));
    printf("RPI sent: %s", msg);

    // 從 ESP32 接收資料（一直收，直到遇到 \n 或超時）
    char buf[256];
    int idx = 0, retry = 0;
    char ch;
    while (retry < 50) {  // 最多等5秒
        int n = read(fd, &ch, 1);
        if (n == 1) {
            buf[idx++] = ch;
            if (ch == '\n' || idx >= sizeof(buf) - 1) {
                buf[idx] = 0;
                printf("RPI received: %s\n", buf);
                break;
            }
        } else {
            usleep(100000); // 0.1秒
            retry++;
        }
    }
    if (idx == 0) {
        printf("No data received from ESP32\n");
    }


    close(fd);
    return 0;
}
