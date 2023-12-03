CC ?= gcc
CFLAGS ?= -ansi -pedantic -Wall -Wextra
CFLAGS_INCLUDE = -I../common/
LDFLAGS ?=

all: succubus_arc_unpack

succubus_arc_unpack: succubus_arc_unpack.c
	$(CC) $(CFLAGS) $(CFLAGS_INCLUDE) -I. $? -o $@ $(LDFLAGS)

clean:
	rm -f succubus_arc_unpack succubus_arc_unpack.exe *.o


.PHONY: clean
