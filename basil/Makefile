CC ?= gcc
CFLAGS ?= -ansi -pedantic -Wall -Wextra
LDFLAGS ?=

all: mif_unpack

mif_unpack: mif_unpack.c
	$(CC) $(CFLAGS) -I. $? -o $@ $(LDFLAGS)

clean:
	rm -f mif_unpack mif_unpack.exe *.o


.PHONY: clean
