CC ?= gcc
CFLAGS ?= -ansi -pedantic -Wall -Wextra
LDFLAGS ?=

all: ifl_unpack

ifl_unpack: ifl_unpack.c
	$(CC) $(CFLAGS) -I. $? -o $@ $(LDFLAGS)

clean:
	rm -f ifl_unpack ifl_unpack.exe *.o


.PHONY: clean
