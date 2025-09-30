// TCRT5000循跡感測器應用層
#include "tcrt5000.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

static int fd = -1;

// 開檔案
int tcrt5000_open(void){
	
	// 檢查是否已開啟
	if(fd >= 0){
		printf("tcrt5000 has already opened\n");
		return 0;
	}

	// 調用 driver 模組
	fd = open("/dev/tcrt5000", O_RDONLY);
	if(fd < 0){
		perror("tcrt5000_open failed");
		return -1;
	}

	// 成功開啟
	printf("tcrt5000 opened successfully\n");
	return 0;
}


// 讀取檔案
int tcrt5000_read(tcrt5000_data *data) {

	// 1.檢查是否已開啟
	if(fd < 0){
		fprintf(stderr, "tcrt5000_read: 裝置尚未開啟\n");
		return -1;
	}

	// 2.讀取檔案
	char buf[16];		// 裝讀取的值
	ssize_t len = read(fd, buf, sizeof(buf) - 1);

	if(len < 0){
		perror("tcrt5000 read failed");
		return -1;
	}

	// 3.結尾加上字串結束符號
	buf[len] = '\0';

	// 4.如果傳回值小於3個值代表有問題
	if(len < 3){	
		fprintf(stderr, "tcrt5000_read: 解析錯誤, buf= '%s'\n", buf);
		return -1;
	}

	data->left = buf[0] - '0';
	data->middle = buf[1] - '0';
	data->right = buf[2] - '0';

	// 成功
	// printf("tcrt5000 read successfully\n");
	return 0;
}


// 關閉檔案
int tcrt5000_close(void){

	// 如果 fd < 0，代表還沒開起，直接回傳成功
	if(fd < 0) return 0;

	// 關閉裝置 (關閉順便做狀態判斷)
	if(close(fd) < 0){
		perror("tcrt5000_close failed");
		return -1;
	}

	// 重製 fd 表示裝置已關閉
	fd = -1;

	printf("tcrt5000 closed successfully\n");
	return 0;
}

