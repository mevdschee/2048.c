CC      ?= gcc
PREFIX  ?= /usr
BINDIR  ?= $(PREFIX)/bin

all: 2048

2048:
	$(CC) $(CFLAGS) --std=c99 2048.c -o 2048

test: 2048
	./2048 test

install: 2048
	mkdir -p $(DESTDIR)$(BINDIR)
	cp -p 2048 $(DESTDIR)$(BINDIR)/2048

clean:
	rm -f 2048

.PHONY: all clean test
