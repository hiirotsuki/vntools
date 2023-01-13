#include <stdio.h>
#include <string.h>

#include "writeint.h"

int main(int argc, char *argv[])
{
	char *p;
	FILE *arc, *entry, *lst;
	char filename[13] = {0};
	long index_offset, data_offset, l;
	unsigned int count = 0, entry_size;
	unsigned char magic[] = {"\x53\x4d\x32\x4d\x50\x58\x31\x30"};

	if(argc < 3)
	{
		fprintf(stderr, "missing arguments\n");
		return 1;
	}

	if(strlen(argv[1]) > 12)
	{
		fprintf(stderr, "archive name is too long, max length is 12 characters\n");
		return 1;
	}

	arc = fopen(argv[1], "wb");
	lst = fopen(argv[2], "rb");

	while(1)
	{
		memset(filename, '\0', 13);
		if(!fgets(filename, 13, lst))
			break;
		count++;
	}
	rewind(lst);

	fwrite(magic, 1, 8, arc);
	fwrite_uint32_le(count, arc);
	fwrite_uint32_le((count * 20) + 32, arc);
	strcpy(filename, argv[1]);
	fwrite(filename, 1, 12, arc);
	fwrite_uint32_le(32, arc);

	data_offset = (count * 20) + 32;
	while(1)
	{
		memset(filename, '\0', 13);
		if(!fgets(filename, 13, lst))
			break;

		if((p = strrchr(filename, '\r')))
			*p = '\0';

		if((p = strrchr(filename, '\n')))
			*p = '\0';

		entry = fopen(filename, "rb");

		if(!entry)
		{
			fprintf(stderr, "could not open %s\n", filename);
			return 1;
		}

		fseek(entry, 0, SEEK_END);
		entry_size = ftell(entry);
		rewind(entry);

		fwrite(filename, 1, 12, arc);
		fwrite_uint32_le(data_offset, arc);
		fwrite_uint32_le(entry_size, arc);

		index_offset = ftell(arc);

		fseek(arc, data_offset, SEEK_SET);

		for(l = 0; l < entry_size; l++)
			fputc(fgetc(entry), arc);

		fseek(arc, index_offset, SEEK_SET);
		data_offset += entry_size;

		fprintf(stdout, "writing %s to archive\n", filename);

		fclose(entry);
	}

	fclose(lst);
	fclose(arc);
	return 0;
}

