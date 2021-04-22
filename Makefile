CFLAGS += -std=c99
PREFIX ?= /usr

.PHONY: all clean test

all: 2048

test: 2048
	./2048 test

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -p 2048 $(DESTDIR)$(PREFIX)/bin/2048

clean:
	rm -f 2048
