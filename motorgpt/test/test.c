#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>    // open
#include <unistd.h>   // close, sleep
#include <string.h>   // strlen

static char *dev = "/dev/motor";

int main()
{
    int fd, ret;

    // 開啟裝置檔案
    fd = open(dev, O_RDWR);
    if (fd < 0) {
        printf("Cannot open device file: %s\n", dev);
        exit(EXIT_FAILURE);
    }

    // 馬達 forward
    ret = write(fd, "forward", strlen("forward"));
    if (ret < 0) {
        perror("write forward fail");
    }
    sleep(2);

    // 馬達 stop
    ret = write(fd, "stop", strlen("stop"));
    if (ret < 0) {
        perror("write stop fail");
    }
    sleep(1);

    // 馬達 backward
    ret = write(fd, "backward", strlen("backward"));
    if (ret < 0) {
        perror("write backward fail");
    }
    sleep(2);

    // 馬達 stop
    ret = write(fd, "stop", strlen("stop"));
    if (ret < 0) {
        perror("write stop fail");
    }

    close(fd);
    return 0;
}

