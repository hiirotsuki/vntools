CC ?= gcc
CFLAGS ?= -ansi -pedantic -Wall -Wextra
LDFLAGS ?=

all: ifl_unpack ifl_pack

ifl_unpack: ifl_unpack.c
	$(CC) $(CFLAGS) -I. $? -o $@ $(LDFLAGS)

ifl_pack: ifl_pack.c
	$(CC) $(CFLAGS) -I. $? -o $@ $(LDFLAGS)

clean:
	rm -f ifl_unpack ifl_unpack.exe ifl_pack ifl_pack.exe *.o


.PHONY: clean
