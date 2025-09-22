TARGET=test

#CROSS_COMPILE=aarch64-linux-gnu-
CC = $(CROSS_COMPILE)gcc
CFLAGS = -g -O2 -Wall

all: $(TARGET)


$(TARGET): $(TARGET).c
	$(CC) $(TARGET).c  -o $(TARGET)

clean:
	rm -f $(TARGET)
