#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>    // open
#include <unistd.h>   // close, sleep
#include <string.h>   // strlen

static char *dev = "/dev/motor";

int main()
{
    int fd, ret;

    // �}�Ҹ˸m�ɮ�
    fd = open(dev, O_RDWR);
    if (fd < 0) {
        printf("Cannot open device file: %s\n", dev);
        exit(EXIT_FAILURE);
    }

    // ���F forward
    ret = write(fd, "forward", strlen("forward"));
    if (ret < 0) {
        perror("write forward fail");
    }
    sleep(2);

    // ���F stop
    ret = write(fd, "stop", strlen("stop"));
    if (ret < 0) {
        perror("write stop fail");
    }
    sleep(1);

    // ���F backward
    ret = write(fd, "backward", strlen("backward"));
    if (ret < 0) {
        perror("write backward fail");
    }
    sleep(2);

    // ���F stop
    ret = write(fd, "stop", strlen("stop"));
    if (ret < 0) {
        perror("write stop fail");
    }

    close(fd);
    return 0;
}

