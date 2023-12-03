CC ?= gcc
CFLAGS ?= -ansi -pedantic -Wall -Wextra
CFLAGS_INCLUDE = -I../common/
LDFLAGS ?=

all: mif_unpack ng3_unpack

mif_unpack: mif_unpack.c
	$(CC) $(CFLAGS) $(CFLAGS_INCLUDE) -I. $? -o $@ $(LDFLAGS)

ng3_unpack: ng3_unpack.c
	$(CC) $(CFLAGS) $(CFLAGS_INCLUDE) -I. $? -o $@ $(LDFLAGS)

clean:
	rm -f mif_unpack mif_unpack.exe ng3_unpack ng3_unpack.exe *.o


.PHONY: clean
