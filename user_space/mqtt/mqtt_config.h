// MQTT  rpi對rpi通訊設定
#ifndef __MQTT_CONFIG_H__
#define __MQTT_CONFIG_H__




// Broker 設定
#define MQTT_BROKER_IP "192.168.69.162"
#define MQTT_BROKER_PORT 9566

// 訊息頻道(TOPIC)
#define MQTT_TOPIC_CAR		"statusMSG/car"		// 車子資訊
#define MQTT_TOPIC_MAP		"statusMSG/map"		// 車子資訊
#define MQTT_TOPIC_PARKING	"statusMSG/parking"		// 車子資訊
#define MQTT_TOPIC_CALLING	"statusMSG/callingCar"		// 車子資訊


// 訂閱訊息的 callback 型態
typedef void (*mqtt_callback)(const char *topic, const char *msg);


// ------------  MQTT api  -----------------

// 訂閱訊息的 callback 型態
int mqtt_init(void);

// 初始化 MQTT Client，連線到 Broker (return 成功0  失敗1)
int mqtt_publish(const char *topic, const char *msg);

// 發布訊息到指定的頻道 topic (return 成功0  失敗1)
int mqtt_subscribe(const char *topic, mqtt_callback cb);

// 訂閱指定頻道 topic，收到訊息會呼叫 callback (return 成功0  失敗1)
int mqtt_close(void);


#endif
















