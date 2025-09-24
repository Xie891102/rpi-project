CC = gcc
INCLUDES = -I../../userspace_includes
CFLAGS = -Wall -pthread $(INCLUDES)
TARGET = test_hcsr04

SRCS = main.c hcsr04.c hcsr04_thread.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o $(TARGET)
