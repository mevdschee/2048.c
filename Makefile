CFLAGS += -std=c99

.PHONY: all clean test install

all: 2048

test: 2048
	./2048 test

clean:
	rm -f 2048

install:
	cp 2048 /usr/local/bin/
	cp -n .2048-score /usr/local/bin/