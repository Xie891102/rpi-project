# Note: The ioctl only works for cross-compiling
#
#CROSS_COMPILE=aarch64-linux-gnu-
CC = $(CROSS_COMPILE)gcc

TARGET=test

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(TARGET).c -static  -o $(TARGET)

clean:
	rm -f $(TARGET)
