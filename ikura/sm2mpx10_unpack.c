#include <stdio.h>
#include <string.h>

#include "readint.h"

int main(int argc, char *argv[])
{
	long arc_pos;
	char filename[13];
	unsigned char buf[32];
	FILE *arc, *entry, *lst;
	unsigned int count, index_size, entry_size;
	unsigned char magic[] = {"\x53\x4d\x32\x4d\x50\x58\x31\x30"};

	if(argc < 2)
	{
		fprintf(stderr, "missing arguments\n");
		return 1;
	}

	arc = fopen(argv[1], "rb");

	if(!arc)
	{
		fprintf(stderr, "could not open file %s\n", argv[1]);
		return 1;
	}

	fread(buf, 1, 32, arc);

	if(memcmp(magic, buf, 8) || strlen(argv[1]) > 12)
	{
		fprintf(stderr, "%s is not an Ikura archive\n", argv[1]);
		return 1;
	}

	count = read_uint32_le(&buf[8]);
	index_size = read_uint32_le(&buf[12]);

	/* :-) */
	memset(buf, '\0', 32);
	strcpy((char *)buf, argv[1]);
	strcat((char *)buf, ".txt");

	lst = fopen((char *)buf, "wb");

	while(count--)
	{
		fread(buf, 1, 20, arc);
		arc_pos = ftell(arc);

		memset(filename, '\0', 13);
		memcpy(filename, buf, 12);
		fprintf(stdout, "extracting %s\n", filename);
		fprintf(lst, "%s\n", filename);
		entry = fopen(filename, "wb");
		entry_size = read_uint32_le(&buf[16]);
		fseek(arc, read_uint32_le(&buf[12]), SEEK_SET);

		while(entry_size--)
			fputc(fgetc(arc), entry);

		fseek(arc, arc_pos, SEEK_SET);
		fclose(entry);
	}

	fclose(lst);
	fclose(arc);
	return 0;
}
