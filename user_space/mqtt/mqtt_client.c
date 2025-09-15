// MQTT  Rpi對Rpi通訊 (初始化連線、發布、訂閱、關閉) api
// 使用 Mosquitto library

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>
#include "mqtt_config.h"




// 全局變數
static struct mosquitto *mosq = NULL;	// Mosquitto client物件
static mqtt_callback user_cb = NULL;	// 使用者設定的 callback 函式


// Mosquitto library 初始化與 Broker 連線
int mqtt_init(void){
	
	int rc;

	// 初始化 mosquitto library
	mosquitto_lib_init();

	// 建立新的 Mosquitto client, NULL隨機clientID, true= clean session
	mosq = mosquitto_new(NULL, true, NULL);
	if(!mosq){
		fprintf(stderr, "Failed to create mosquitto instance\n");
		return -1;
	}	

	// 連線到broker
	rc = mosquitto_connect(mosq, MQTT_BROKER_IP, MQTT_BROKER_PORT, 60);
	if(rc != MOSQ_ERR_SUCCESS){
		fprintf(stderr, "Failed to connect to broker: %s\n", mosquitto_strerror(rc));
		mosquitto_destroy(mosq);
		return -1;
	}

	// 啟動背景 thread，處理訂閱接收
	rc = mosquitto_loop_start(mosq);
	if(rc != MOSQ_ERR_SUCCESS){
		fprintf(stderr, "Failed to start mosquitto loop: %s\n", mosquitto_strerror(rc));
		mosquitto_destroy(mosq);
		return -1;
	}

	return 0;
}


// 發布訊息
int mqtt_publish(const char *topic, const char *msg){
	
	// 失敗直接結束
	if(!mosq) return -1;

	int rc = mosquitto_publish(mosq, NULL, topic, strlen(msg), msg, 0, false);
	if(rc != MOSQ_ERR_SUCCESS){
		fprintf(stderr, "Failed to publish message: %s\n", mosquitto_strerror(rc));
		return -1;
	}
	return 0;
}


// Mossquitto callback，收到訊息時會呼叫 (mqtt_subscribe使用)
static void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message){
	if(user_cb && message->payloadlen > 0){
		// 將 topic 與 payload 傳給使用者的 callback
		char msg[message->payloadlen + 1];

		memcpy(msg, message->payload, message->payloadlen);
		msg[message->payloadlen] = '\0';	// 確保字串結尾
		user_cb(message->topic, msg);
	}
}


// 訂閱主題
int mqtt_subscribe(const char *topic, mqtt_callback cb){
	
	// 如果失敗 退出
	if(!mosq) return -1;

	// 設定使用者 callback
	user_cb = cb;

	// 設定 Mosquitto library 的 message callback
	mosquitto_message_callback_set(mosq, on_message);
	
	// 訂閱 topic
	int rc = mosquitto_subscribe(mosq, NULL, topic, 0);
	if(rc != MOSQ_ERR_SUCCESS){
		fprintf(stderr, "Failed to subscribe topic: %s\n", mosquitto_strerror(rc));
		return -1;
	}

	return 0;
}


// 關閉 MQTT client
int mqtt_close(void){
	
	// 如果失敗 退出
	if(!mosq) return -1;

	// 斷線
	mosquitto_disconnect(mosq);

	// 停止背景 loop
	mosquitto_loop_stop(mosq, true);
	
	// 銷毀 client 物件
	mosquitto_destroy(mosq);
	mosq = NULL;

	// 清理 library
	mosquitto_lib_cleanup();		

	return 0;
}






