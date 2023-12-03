CC ?= gcc
CFLAGS ?= -ansi -pedantic -Wall -Wextra
CFLAGS_INCLUDE = -I../common/
LDFLAGS ?=

all: opk_unpack opk_pack

opk_unpack: opk_unpack.c
	$(CC) $(CFLAGS) $(CFLAGS_INCLUDE) -I. $? -o $@ $(LDFLAGS)

opk_pack: opk_pack.c
	$(CC) $(CFLAGS) $(CFLAGS_INCLUDE) -I. $? -o $@ $(LDFLAGS)

clean:
	rm -f opk_unpack opk_unpack.exe opk_pack opk_pack.exe *.o


.PHONY: clean
