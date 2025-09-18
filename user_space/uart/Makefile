CC = gcc
CFLAGS = -Wall -pthread

TARGET = test
OBJS = test.o uart.o uart_thread.o uart_queue.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)











