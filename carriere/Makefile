CC ?= gcc
CFLAGS ?= -ansi -pedantic -Wall -Wextra
LDFLAGS ?=

all: cgd2bmp

cgd2bmp: cgd2bmp.c
	$(CC) $(CFLAGS) -I. $? -o $@ $(LDFLAGS)

clean:
	rm -f cgd2bmp cgd2bmp.exe *.o


.PHONY: clean
