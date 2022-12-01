CC ?= gcc
CFLAGS ?= -ansi -pedantic -Wall -Wextra
LDFLAGS ?=

all: nekopunch_unpack

nekopunch_unpack: nekopunch_unpack.c cp932.c cp932table.c win32compat.c
	$(CC) $(CFLAGS) -I. $? -o $@ $(LDFLAGS)

clean:
	rm -f nekopunch_unpack nekopunch_unpack.exe *.o


.PHONY: clean
