// 與 調度中心 nodejs測試連線
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mqtt_config.h"
#include "../route/route.h"

// 全局路線指標
Route *current_route = NULL;  

// 收到訊息 json 的 call back
void car_message_cb(const char *topic, const char *msg){
	printf("Received on %s: %s\n", topic, msg);

	// 解析 Json 格式中 route 陣列，將數字存到raw_steps
	int raw_steps[15];	// 最多 15 步	
	int count = 0;

	// 找到 "route":[ 後開始解析
	const char *p = strstr(msg, "\"route\":[");
	if(p){
		p += 9;		// 跳過 "route":[
		while(*p && *p != ']' && count < 10){
			if(*p >= '1' && *p <= '4'){
				// 字元轉數字
				raw_steps[count++] = *p - '0';
			}
			p++;
		}
	}

	if(count == 0){
		printf("No valid steps found.\n");
		return; 
	}

	// 建立路線
	Route *route = create_route(raw_steps, count);

	// 印出每一步文字
	printf("Pared route steps: \n");
	for(int i = 0; i < route->length; i++){
		printf("Steps %d: %s\n", i+1, action_to_string(route->steps[i]));
	}
	
	// 回復調度中心 已收到訊號了
	mqtt_publish("statusMSG/car", "車體已收到路線指示");

	free_route(route);
}


// 主程式
int main(){

	// 1.初始化
	if(mqtt_init() != 0){
		printf("MQTT init failed\n");
		return -1;
	}

	// 2.先訂閱車子訊息
	mqtt_subscribe("statusMSG/mapTest/routes", car_message_cb);

	// 3.發送車子訊息到broker
	mqtt_publish("statusMSG/mapTest/parking", "parking:1");
	printf("Message sent.\n");	

	// 4.保持車體運行
	printf("Waiting for messages... Press Ctrl+C to exit\n");
	while(1){
		sleep(1);	// 避免cpu空轉
	}

	// 5.關閉 mqtt client
	mqtt_close();
	printf("MQTT client closed.\n");

	return 0;
}



