#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "readint.h"
#include "writeint.h"

struct color
{
	unsigned char b;
	unsigned char g;
	unsigned char r;
};

int main(int argc, char *argv[])
{
	char *p;
	FILE *ng3, *bmp;
	char filename[255];
	unsigned int width;
	unsigned int height;
	unsigned char buf[12];
	struct color colormap[256];
	int i, stride, pixels, written;

	if(argc < 2)
	{
		fprintf(stderr, "usage: ng3_unpack: <file.ng3>\n");
		return 1;
	}

	strcpy(filename, argv[1]);
	p = strrchr(filename, '.');
	if(p == NULL)
	{
		fprintf(stderr, "expected NG3 image file\n");
		return 1;
	}

	if(memcmp(p, ".ng3", 4))
	{
		fprintf(stderr, "expected NG3 image file\n");
		return 1;
	}

	memcpy(p, ".bmp", 4);

	ng3 = fopen(argv[1], "rb");
	bmp = fopen(filename, "wb");

	if(ng3 == NULL)
	{
		fprintf(stderr, "failed to open %s for reading: %s\n", argv[1], strerror(errno));
		return 1;
	}

	if(fread(buf, 1, 12, ng3) != 12)
	{
		fprintf(stderr, "unexpected end of file\n");
		return 1;
	}

	if(memcmp(buf, "NG3\0", 4))
	{
		fprintf(stderr, "expected NG3 image file\n");
		return 1;
	}

	width = read_uint32_le(&buf[4]);
	height = read_uint32_le(&buf[8]);

	/* todo? error handling */
	for(i = 0; i < 256; i++)
	{
		colormap[i].b = fgetc(ng3);
		colormap[i].g = fgetc(ng3);
		colormap[i].r = fgetc(ng3);
	}

	stride = width * 3;
	pixels = stride * height;

	/* BMP header, enough for 24-bit BMPs */
	fwrite("BM", 1, 2, bmp);
	fwrite_uint32_le(54 + pixels, bmp);
	fwrite("\x00\x00\x00\0x00", 4, 1, bmp);
	fwrite_uint32_le(54, bmp);
	fwrite_uint32_le(40, bmp);
	fwrite_uint32_le(width, bmp);
	fwrite_uint32_le(height, bmp);
	fwrite("\x01\x00", 2, 1, bmp);
	fwrite("\x18\x00", 2, 1, bmp);
	fwrite("\x00\x00\x00\0x00", 4, 1, bmp);
	fwrite_uint32_le(pixels, bmp);
	fwrite("\x00\x00\x00\0x00", 4, 1, bmp); /* unused */
	fwrite("\x00\x00\x00\0x00", 4, 1, bmp); /* unused */
	fwrite("\x00\x00\x00\0x00", 4, 1, bmp);
	fwrite("\x00\x00\x00\0x00", 4, 1, bmp);

	written = 0;
	while(1)
	{
		int c;
		if(written >= pixels)
			break;
		c = fgetc(ng3);
		ungetc(c, ng3);
		if(c == -1)
			break;
		if(c == 1)
		{
			int index;
			fgetc(ng3);
			index = fgetc(ng3);
			fputc(colormap[index].b, bmp);
			fputc(colormap[index].g, bmp);
			fputc(colormap[index].r, bmp);
			written += 3;
		}
		else if(c == 2)
		{
			int index, count;
			fgetc(ng3);
			index = fgetc(ng3);
			count = fgetc(ng3);
			while(count--)
			{
				fputc(colormap[index].b, bmp);
				fputc(colormap[index].g, bmp);
				fputc(colormap[index].r, bmp);
				written += 3;
			}
		}
		else
		{
			fputc(fgetc(ng3), bmp);
			fputc(fgetc(ng3), bmp);
			fputc(fgetc(ng3), bmp);
			written += 3;
		}
	}

	return 0;
}
