# 測試 TCRT5000 循跡紅線
CC = gcc
CFLAGS = -Wall -g -I..
TARGET = test_tcrt5000

all: $(TARGET)

$(TARGET): test_tcrt5000.o tcrt5000.o tcrt5000_thread.o
	$(CC) $(CFLAGS) -o $(TARGET) test_tcrt5000.o tcrt5000.o tcrt5000_thread.o -lpthread

test.o: test.c ../includes.h
	$(CC) $(CFLAGS) -c test.c

tcrt5000.o: tcrt5000.c ../includes.h
	$(CC) $(CFLAGS) -c tcrt5000.c

tcrt5000_thread.o: tcrt5000_thread.c ../includes.h
	$(CC) $(CFLAGS) -c tcrt5000_thread.c

clean:
	rm -f *.o $(TARGET)















