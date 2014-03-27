CFLAGS ?= -std=c99

.PHONY: all

all: 2048

2048: 2048.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f 2048