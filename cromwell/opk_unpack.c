#include <stdio.h>
#include <string.h>

#include "readbytes.h"

int main(int argc, char *argv[])
{
	char filename[8 + 4]; /* + .ogg */
	unsigned char buf[20];
	FILE *opk, *ogg, *lst;
	long count, name_pos, index_pos;
	const char *magic = "VoiceOggPackFile";
	unsigned int entry_offset, entry_next, entry_size;

	if(argc < 2)
		return 1;

	if(argc >= 3)
		lst = fopen(argv[2], "wb");
	else
		lst = fopen("list.txt", "wb");

	opk = fopen(argv[1], "rb");

	if(!opk)
	{
		fprintf(stderr, "could not open file %s\n", argv[1]);
		return 1;
	}

	fread(buf, 1, 20, opk);

	if(memcmp(magic, buf, 16))
	{
		fprintf(stderr, "%s is not a cromwell audio archive\n", argv[1]);
		return 1;
	}

	count = read_uint32_le(&buf[16]);

	index_pos = ftell(opk);
	fseek(opk, -(count * 8), SEEK_END);
	name_pos = ftell(opk);

	fseek(opk, 20, SEEK_SET);
	fread(buf, 1, 4, opk);
	entry_next = read_uint32_le(buf);
	index_pos = ftell(opk);
	while(count--)
	{
		fseek(opk, name_pos, SEEK_SET);
		fread(filename, 1, 8, opk);
		strcat(filename, ".ogg");
		fprintf(stdout, "extracting %s\n", filename);
		name_pos += 8;
		ogg = fopen(filename, "wb");
		fprintf(lst, "%s\n", filename);

		fseek(opk, index_pos, SEEK_SET);
		entry_offset = entry_next;
		fread(buf, 1, 4, opk);
		entry_next = read_uint32_le(buf);
		entry_size = entry_next - entry_offset;

		index_pos = ftell(opk);
		fseek(opk, entry_offset, SEEK_SET);

		while(entry_size--)
			fputc(fgetc(opk), ogg);

		fseek(opk, index_pos, SEEK_SET);
		fclose(ogg);
	}

	fclose(lst);
	fclose(opk);
	return 0;
}
