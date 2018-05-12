CC=gcc
CFLAGS= -std=gnu99 -Wall

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

all: main

main: main.o
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o
	rm -f main
