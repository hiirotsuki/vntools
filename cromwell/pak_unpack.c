#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

#include "readint.h"

int main(int argc, char *argv[])
{
	char *p;
	long index;
	int audio = 0;
	FILE *pak, *out;
	char filename[16];
	char *magic = "Graphic PackData";
	unsigned long count, offset, next_offset, uncompressed_filesize;
	unsigned char buf[36], nbuf[16];

	if(argc < 2)
		return 1;

	pak = fopen(argv[1], "rb");

	if(!pak)
	{
		fprintf(stderr, "could not open file: %s\n", argv[1]);
		return 1;
	}

	if(fread(buf, 1, 20, pak) != 20)
	{
		fprintf(stderr, "could not read file %s\n", argv[1]);
		return 1;
	}

	if(memcmp(buf, magic, 16))
	{
		fprintf(stderr, "%s is not a cromwell pak archive\n", argv[1]);
		return 1;
	}

	count = read_uint32_le(&buf[16]);

	if(strlen(argv[1]) >= 9)
		if((p = strrchr(argv[1], '.')))
			audio = (strncmp(p - 9, "VOICE", 5)) ? 1 : 0;

	while(count--)
	{
		unsigned long filesize;
		unsigned char *cbuf, *ucbuf;

		/* filenames are not C strings */
		fread(buf, 1, 20, pak);
		memset(filename, '\0', 16);
		memcpy(filename, buf, 12);

		fprintf(stdout, "extracting %s\n", filename);

		if((p = strrchr(filename, '.')))
		{
			if(!strcmp(p, ".prs"))
				if(!audio)
					strcat(filename, ".bmp");
				else
					strcat(filename, ".wav");
		}

		offset = read_uint32_le(&buf[12]);
		uncompressed_filesize = read_uint32_le(&buf[16]);
		index = ftell(pak);
		if(count > 0)
		{
			fread(nbuf, 1, 16, pak);
			next_offset = read_uint32_le(&nbuf[12]);
			fseek(pak, -16, SEEK_CUR);
		}
		else
		{
			fseek(pak, 0, SEEK_END);
			next_offset = ftell(pak);
		}
		/*fprintf(stdout, "filename %s - offset %d - next offset %d\n", filename, offset, next_offset);*/
		fseek(pak, offset, SEEK_SET);
		filesize = next_offset - offset;

		cbuf = malloc(filesize);
		ucbuf = malloc(uncompressed_filesize);

		if(!cbuf || !ucbuf)
		{
			fprintf(stderr, "failed to allocate memory, exiting...\n");
			return 1;
		}

		fread(cbuf, 1, filesize, pak);

		if(uncompress(ucbuf, &uncompressed_filesize, cbuf, filesize) != Z_OK)
		{
			fprintf(stderr, "failed to decompress entry, exiting...\n");
			return 1;
		}

		out = fopen(filename, "wb");

		fwrite(ucbuf, 1, uncompressed_filesize, out);

		free(cbuf);
		free(ucbuf);
		fclose(out);
		fseek(pak, index, SEEK_SET);
	}

	return 0;
}
