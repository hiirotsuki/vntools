CC ?= gcc
CFLAGS ?= -ansi -pedantic -Wall -Wextra
CFLAGS_INCLUDE = -I../common/
LDFLAGS ?=

all: sm2mpx10_unpack ggp_decrypt

sm2mpx10_unpack: sm2mpx10_unpack.c
	$(CC) $(CFLAGS) $(CFLAGS_INCLUDE) -I. $? -o $@ $(LDFLAGS)

ggp_decrypt: ggp_decrypt.c
	$(CC) $(CFLAGS) $(CFLAGS_INCLUDE) -I. $? -o $@ $(LDFLAGS)

clean:
	rm -f sm2mpx10_unpack sm2mpx10_unpack.exe ggp_decrypt ggp_decrypt.exe *.o


.PHONY: clean
