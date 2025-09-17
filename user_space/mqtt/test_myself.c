// 測試 MQTT

#include <stdio.h>
#include <unistd.h>
#include "mqtt_config.h"


// callback
void my_callback(const char* topic, const char* msg){
	printf("[RECEIVE] topic: %s, msg: %s\n", topic, msg);
}


int main(){

	// 初始化
	if(mqtt_init() != 0){
		printf("MQTT init failed\n");
		return -1;
	}

	// 訂閱 topic
	mqtt_subscribe(MQTT_TOPIC_CAR, my_callback);

	int count = 0;
	while(count < 10) {
		char buf[50];
		sprintf(buf, "Hello #%d", count);

		// 發布消息
		mqtt_publish(MQTT_TOPIC_CAR, buf);

		printf("Main loop running: %d\n",count);

		sleep(1);	// 主程式繼續做其他事情
		count++;
	}

	mqtt_close();
	return 0;
}














