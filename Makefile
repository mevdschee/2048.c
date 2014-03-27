IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR) -std=c99

ODIR=.
LDIR =../lib

LIBS=

_DEPS = 2048.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = 2048.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

2048: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

all: 2048

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 2048
