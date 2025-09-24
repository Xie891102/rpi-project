#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int fd = open("/dev/ultrasonic0", O_RDONLY);
    if(fd < 0){
        perror("open failed");
        return -1;
    }

    char buf[16];
    ssize_t len = read(fd, buf, sizeof(buf)-1);
    if(len > 0){
        buf[len] = '\0';
        printf("Distance: %s\n", buf);
    } else {
        printf("Read failed or no data\n");
    }

    close(fd);
    return 0;
}
