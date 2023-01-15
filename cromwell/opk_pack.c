#include <stdio.h>
#include <string.h>

#include "writeint.h"

int main(int argc, char *argv[])
{
	char *p;
	int count, i;
	char filename[13];
	FILE *opk, *ogg, *lst;
	const char *magic = "VoiceOggPackFile";
	long name_offset, data_offset, ogg_size, l;

	if(argc < 2)
	{
		fprintf(stderr, "usage: opk_pack <archive.opk> [optional] <list.txt>\n");
		return 1;
	}

	if(argc >= 3)
		lst = fopen(argv[2], "rb");
	else
		lst = fopen("list.txt", "rb");

	if(!lst)
	{
		if(argc >= 3)
			fprintf(stderr, "could not open list file %s\n", argv[2]);
		else
			fprintf(stderr, "could not open list.txt file\n");
		return 1;
	}

	count = 0;
	while(1)
	{
		memset(filename, '\0', 13);
		if(!fgets(filename, 13, lst))
			break;
		count++;
	}
	rewind(lst);

	opk = fopen(argv[1], "wb");

	fputs(magic, opk);
	fwrite_uint32_le(count, opk);

	data_offset = (count * 4) + 24;

	fwrite_uint32_le(data_offset, opk);

	for(i = 0; i <= count; i++)
	{
		memset(filename, '\0', 13);
		if(!fgets(filename, 13, lst))
			break;

		if((p = strrchr(filename, '\r')))
			*p = '\0';
		if((p = strrchr(filename, '\n')))
			*p = '\0';

		ogg = fopen(filename, "rb");
		if(!ogg)
		{
			fprintf(stderr, "could not open file %s\n", filename);
			return 1;
		}

		fprintf(stdout, "writing %s to archive\n", filename);

		fseek(ogg, 0, SEEK_END);
		ogg_size = ftell(ogg);
		rewind(ogg);

		fseek(opk, (i * 4) + 20, SEEK_SET);
		fwrite_uint32_le(data_offset, opk);

		fseek(opk, data_offset, SEEK_SET);
		for(l = 0; l < ogg_size; l++)
			fputc(fgetc(ogg), opk);

		data_offset += ogg_size;

		fclose(ogg);
	}

	fseek(opk, 0, SEEK_END);
	name_offset = ftell(opk);
	fseek(opk, (i * 4) + 20, SEEK_SET);
	fwrite_uint32_le(name_offset, opk);

	rewind(lst);
	fseek(opk, 0, SEEK_END);
	while(count--)
	{
		char buf[9];
		memset(buf, '\0', 9);
		memset(filename, '\0', 13);
		if(!fgets(filename, 13, lst))
			break;

		if((p = strrchr(filename, '.')))
			*p = '\0';

		strcpy(buf, filename);

		fwrite(buf, 1, 8, opk);
	}

	fclose(lst);
	fclose(opk);
	return 0;
}
