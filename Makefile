CFLAGS += -std=c99
CC = cc
UNAME = $(shell uname)
PREFIX ?= /usr

ifeq "$(UNAME)" "Haiku"
	CC = gcc-x86
endif

.PHONY: all clean test

2048:	
	$(CC) $(CFLAGS) 2048.c -o 2048

all: 2048

test: 2048
	./2048 test

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -p 2048 $(DESTDIR)$(PREFIX)/bin/2048

clean:
	rm -f 2048
