CC ?= gcc
CFLAGS ?= -ansi -pedantic -Wall -Wextra
LDFLAGS ?=

all: ggp_decrypt

ggp_decrypt: ggp_decrypt.c
	$(CC) $(CFLAGS) -I. $? -o $@ $(LDFLAGS)

clean:
	rm -f ggp_decrypt ggp_decrypt.exe *.o


.PHONY: clean
