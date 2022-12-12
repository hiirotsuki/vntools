#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "readbytes.h"

int main(int argc, char *argv[])
{
	char *p;
	char png_name[1024];
	FILE *ggp_file, *png_file, *region_file;
	unsigned int data_offset, data_size, region_offset, region_size, i;
	unsigned char key[8], buf[36], magic[] = {"\x47\x47\x50\x46\x41\x49\x4b\x45"};

	if(argc < 2)
	{
		fprintf(stderr, "missing arguments\n");
		return 1;
	}

	ggp_file = fopen(argv[1], "rb");

	if(!ggp_file)
	{
		fprintf(stderr, "could not open file %s\n", argv[1]);
		return 1;
	}

	fread(buf, 1, 36, ggp_file);

	if(memcmp(magic, buf, 8) || !strrchr(argv[1], '.'))
	{
		fprintf(stderr, "not a GGP file");
		return 1;
	}

	for(i = 0; i < 8; i++)
		key[i] = buf[i] ^ buf[i + 12];

	data_offset   = read_uint32_le(&buf[20]);
	data_size     = read_uint32_le(&buf[24]);
	region_offset = read_uint32_le(&buf[28]);
	region_size   = read_uint32_le(&buf[32]);

	fseek(ggp_file, data_offset, SEEK_SET);
	strcpy(png_name, argv[1]);
	p = strrchr(png_name, '.');
	p[1] = 'p';
	p[2] = 'n';
	p[3] = 'g';
	p[4] = '\0';

	png_file = fopen(png_name, "wb");
	for(i = 0; i < data_size; i++)
		fputc(fgetc(ggp_file) ^ key[i % 8], png_file);

	fclose(png_file);

	if(region_size)
	{
		fprintf(stdout, "region chunk is not empty\n");
		fseek(ggp_file, region_offset, SEEK_SET);

		p = strrchr(png_name, '.');
		p[1] = 'r';
		p[2] = 'e';
		p[3] = 'g';
		p[4] = 'i';
		p[5] = 'o';
		p[6] = 'n';
		p[7] = '\0';

		region_file = fopen(png_name, "wb");

		while(region_size--)
			fputc(fgetc(ggp_file), region_file);

		fclose(region_file);
	}

	fclose(ggp_file);
	return 0;
}
