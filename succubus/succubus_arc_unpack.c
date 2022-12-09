#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "readbytes.h"

int main(int argc, char *argv[])
{
	long arc_pos;
	char filename[21];
	int is_mp3_archive = 0;
	FILE *arc_file, *out_file;
	unsigned int entry_size, num;
	unsigned char buf[24], magic[] = {"\x41\x52\x43\x31"};

	if(argc != 2)
	{
		fprintf(stderr, "missing arguments\n");
		return 1;
	}

	arc_file = fopen(argv[1], "rb");
	if(!arc_file)
	{
		fprintf(stderr, "could not open file %s\n", argv[1]);
		return 1;
	}

	/* TODO: detect and handle strange ogg archives */
	if(!strcmp(argv[1], "voice.arc"))
		is_mp3_archive = 1;

	fread(buf, 1, 12, arc_file);
	if(memcmp(magic, buf, 4))
	{
		fprintf(stderr, "not a succubus archive");
		return 1;
	}

	num = read_uint32_le(&buf[4]);
	arc_pos = read_uint32_le(&buf[8]);

	while(num--)
	{
		memset(filename, '\0', sizeof(filename));
		fseek(arc_file, arc_pos, SEEK_SET);
		fread(buf, 1, 24, arc_file);
		arc_pos = ftell(arc_file);
		memcpy(filename, buf, 16);

		if(is_mp3_archive)
			strcat(filename, ".mp3");

		fprintf(stdout, "extracting %s\n", filename);

		out_file = fopen(filename, "wb");

		entry_size = read_uint32_le(&buf[16]);
		fseek(arc_file, read_uint32_le(&buf[20]), SEEK_SET);
		while(entry_size--)
			fputc(fgetc(arc_file), out_file);

		fclose(out_file);
		fseek(arc_file, arc_pos, SEEK_SET);
	}

	fclose(arc_file);
	return 0;
}
