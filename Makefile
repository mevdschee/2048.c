CC=gcc
CFLAGS = -O3 -g3 -std=c99 -Wall -pedantic

.PHONY: all

all: 2048

2048: 2048.o
	$(CC) $(CFLAGS) $^ -o $@ 

clean:
	rm -f 2048 *.o
