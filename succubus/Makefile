CC ?= gcc
CFLAGS ?= -ansi -pedantic -Wall -Wextra
LDFLAGS ?=

all: succubus_arc_unpack

succubus_arc_unpack: succubus_arc_unpack.c
	$(CC) $(CFLAGS) -I. $? -o $@ $(LDFLAGS)

clean:
	rm -f succubus_arc_unpack succubus_arc_unpack.exe *.o


.PHONY: clean
