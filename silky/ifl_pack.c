#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "writeint.h"

int main(int argc, char *argv[])
{
	char *p;
	char filename[18];
	long ifl_pos, entry_size, l;
	unsigned int count, data_offset;
	unsigned char magic[] = {"\x49\x46\x4c\x53"};
	FILE *ifl, *entry, *lst;

	if(argc < 3)
	{
		fprintf(stderr, "usage: ifl_pack <archive.ifl> <list.txt>\n");
		return 1;
	}

	lst = fopen(argv[2], "rb");
	ifl = fopen(argv[1], "wb");

	/* IFL magic */
	fwrite(magic, 1, 4, ifl);

	while(1)
	{
		memset(filename, '\0', 18);
		if(!fgets(filename, 18, lst))
			break;
		count++;
	}
	rewind(lst);

	/* data offset */
	data_offset = (count * 24) + 12;
	write_uint32_le(data_offset, ifl);

	/* file count */
	write_uint32_le(count, ifl);

	while(1)
	{
		memset(filename, '\0', 18);
		if(!fgets(filename, 18, lst))
			break;

		if((p = strrchr(filename, '\r')))
			*p = '\0';

		if((p = strrchr(filename, '\n')))
			*p = '\0';

		entry = fopen(filename, "rb");

		if(!entry)
		{
			fprintf(stderr, "%s doesn't exist\n", filename);
			return 1;
		}

		fseek(entry, 0, SEEK_END);
		entry_size = ftell(entry);
		rewind(entry);

		fwrite(filename, 1, 16, ifl);
		write_uint32_le(data_offset, ifl);

		ifl_pos = ftell(ifl);

		fprintf(stdout, "writing %s to archive\n", filename);

		fseek(ifl, data_offset, SEEK_SET);
		for(l = 0; l < entry_size; l++)
			fputc(fgetc(entry), ifl);

		fseek(ifl, ifl_pos, SEEK_SET);
		data_offset += entry_size;
		write_uint32_le(entry_size, ifl);
		fclose(entry);
	}

	fclose(lst);
	fclose(ifl);
	return 0;
}
