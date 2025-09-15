CC = gcc
CFLAGS = -Wall -g

all: test_route

test_route: main.o route.o
	$(CC) $(CFLAGS) -o test_route main.o route.o

main.o: main.c route.h
	$(CC) $(CFLAGS) -c main.c

route.o: route.c route.h
	$(CC) $(CFLAGS) -c route.c

clean:
	rm -f *.o test_route



