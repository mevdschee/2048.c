CFLAGS += -std=c99

.PHONY: all clean test

all: 2048

test: 2048
	./2048 test

clean:
	rm -f 2048
