#include <stdio.h>
#include <stdlib.h>

#include "readint.h"
#include "writeint.h"

int main(int argc, char *argv[])
{
	char *p;
	long size;
	FILE *cgd, *bmp;
	int width, height;
	char filename[255];
	unsigned char buf[20];

	if(argc < 2)
	{
		return 1;
	}

	/* lowest common denominator */
	/* this shouldn't happen but alas... */
	if(strlen(argv[1]) > 255)
	{
		fprintf(stderr, "path and/or filename exceeds maximum length\n");
		return 1;
	}

	strcpy(filename, argv[1]);
	p = strrchr(filename, '.');
	if(p == NULL)
	{
		fprintf(stderr, "expected cgd file\n");
		return 1;
	}

	if(strcmp(p, ".cgd"))
	{
		fprintf(stderr, "expected cgd file\n");
		return 1;
	}

	memcpy(p, ".bmp", 4);

	cgd = fopen(argv[1], "rb");

	if(cgd == NULL)
	{
		fprintf(stderr, "failed to open %s\n", argv[1]);
		return 1;
	}

	fseek(cgd, 0, SEEK_END);
	size = ftell(cgd);
	fseek(cgd, 0, SEEK_SET);

	if(fread(buf, 1, 20, cgd) != 20)
	{
		fprintf(stderr, "unexpected end of file\n");
		return 1;
	}

	if(memcmp(buf, "cgd\0", 4))
	{
		fprintf(stderr, "not a cgd image file\n");
		return 1;
	}

	width = read_uint32_le(&buf[12]);
	height = read_uint32_le(&buf[16]);

	bmp = fopen(filename, "wb");

	if(bmp == NULL)
	{
		fprintf(stderr, "failed to open %s for writing\n", filename);
		return 1;
	}

	/* BMP header construction */
	fwrite("BM", 1, 2, bmp);
	fwrite_uint32_le(122 + size, bmp);
	fwrite("\x00\x00\x00\0x00", 4, 1, bmp); /* unused */
	fwrite_uint32_le(122, bmp);
	fwrite_uint32_le(108, bmp);
	fwrite_uint32_le(width, bmp);
	fwrite_int32_le(-height, bmp);
	fwrite("\x01\x00", 2, 1, bmp);
	fwrite("\x20\x00", 2, 1, bmp);
	fwrite("\x03\x00\x00\x00", 4, 1, bmp);
	fwrite_uint32_le(size, bmp);
	fwrite("\x00\x00\x00\x00", 4, 1, bmp); /* not necessary */
	fwrite("\x00\x00\x00\x00", 4, 1, bmp); /* not necessary */
	fwrite("\x00\x00\x00\x00", 4, 1, bmp);
	fwrite("\x00\x00\x00\x00", 4, 1, bmp);
	fwrite("\x00\x00\xFF\x00", 4, 1, bmp); /* B */
	fwrite("\x00\xFF\x00\x00", 4, 1, bmp); /* G */
	fwrite("\xFF\x00\x00\x00", 4, 1, bmp); /* R */
	fwrite("\x00\x00\x00\xFF", 4, 1, bmp); /* A */
	fseek(bmp, 122, SEEK_SET);
	while(size--)
		fputc(fgetc(cgd), bmp);

	return 0;
}
