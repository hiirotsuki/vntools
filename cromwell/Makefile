CC ?= gcc
CFLAGS ?= -ansi -pedantic -Wall -Wextra
LDFLAGS ?=

all: opk_unpack

opk_unpack: opk_unpack.c
	$(CC) $(CFLAGS) -I. $? -o $@ $(LDFLAGS)

clean:
	rm -f opk_unpack opk_unpack.exe *.o


.PHONY: clean
