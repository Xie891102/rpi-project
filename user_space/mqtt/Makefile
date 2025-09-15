# MQTT 通訊測試

CC = gcc
CFLAGS = -Wall -O2
LIBS = -lmosquitto

TARGET = test_mqtt
SRCS = test_mqtt.c mqtt_client.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LIBS)


clean:
	rm -f $(TARGET)











