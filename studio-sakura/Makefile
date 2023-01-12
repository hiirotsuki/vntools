CC ?= gcc
CFLAGS ?= -ansi -pedantic -Wall -Wextra
LDFLAGS ?=

all: studio-sakura_unpack

studio-sakura_unpack: studio-sakura_unpack.c cp932.c cp932table.c win32compat.c
	$(CC) $(CFLAGS) -I. $? -o $@ $(LDFLAGS)

clean:
	rm -f studio-sakura_unpack studio-sakura_unpack.exe *.o


.PHONY: clean
