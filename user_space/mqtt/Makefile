# MQTT 通訊測試

CC = gcc
CFLAGS = -Wall -O2
LIBS = -lmosquitto

# 已編譯過的執行檔物件
ROUTE_OBJ = ../route/route.o

# 目標檔案
TARGETS = test_rpi test_myself

all: $(TARGETS)

# 每個程式對應的原始碼
test_myself: test_myself.c mqtt_client.c
	$(CC) $(CFLAGS) -o $@ test_myself.c mqtt_client.c $(LIBS)

test_rpi: test_rpi.c mqtt_client.c $(ROUTE_OBJ)
	$(CC) $(CFLAGS) -o $@ test_rpi.c mqtt_client.c $(ROUTE_OBJ) $(LIBS)


clean:
	rm -f $(TARGETS)











