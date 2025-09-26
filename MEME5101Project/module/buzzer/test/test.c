#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>	//open
#include<unistd.h>	//exit
#include<sys/ioctl.h>	//ioctl
#include "../include/buzzer_ioctl.h"

int main(int argc, char *argv[])
{
	int i, fd, retval;

	fd = open(DEVICE_FILE_NAME, O_RDWR);
	printf("DEVICE_FILE_NAME:%s, fd:%d\n",DEVICE_FILE_NAME, fd);
	if( fd < 0 ) {
		printf("Can't open device file: %s\n",DEVICE_FILE_NAME);
		exit(-1);
	}

	printf("Turn the BUZZER 4 ON\n");
	retval = ioctl(fd, IOCTL_SET_BUZZER_ON, 6);
	if ( retval < 0 ) {
		printf("IOCTL_SET_BUZZER_ON\n");
		exit(-1);
	}
#if 1   /* Turn the other GPIO low */
	for (i=0; i<11; i++) {
		if( i != 4) // Except GPIO5
			ioctl(fd, IOCTL_SET_BUZZER_OFF, i );
	}
#endif
	sleep(1);
	for (i=0; i<11; i++) {
			ioctl(fd, IOCTL_SET_BUZZER_OFF, i );
	}
	close(fd);

	return 0;
}
