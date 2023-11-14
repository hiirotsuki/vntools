#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "readint.h"

int main(int argc, char *argv[])
{
	FILE *mif, *out;
	long arc_offset;
	char filename[17];
	unsigned char buf[24];
	int count, entry_offset, entry_size;

	if(argc < 2)
	{
		fprintf(stderr, "usage: mif_unpack <archive.MIF>\n");
		return 1;
	}

	mif = fopen(argv[1], "rb");

	if(mif == NULL)
	{
		fprintf(stderr, "failed to open %s: %s\n", argv[1], strerror(errno));
		return 1;
	}

	if(fread(buf, 1, 8, mif) != 8)
	{
		fprintf(stderr, "unexpected end of archive\n");
		return 1;
	}

	if(memcmp(buf, "MIF\0", 4))
	{
		fprintf(stderr, "expected MIF archive\n");
		return 1;
	}

	count = read_uint32_le(&buf[4]);
	while(count--)
	{
		fread(buf, 1, 24, mif);
		memset(filename, '\0', 17); /* C string? not sure, lets be conservative */
		memcpy(filename, buf, 16);
		entry_offset = read_uint32_le(&buf[16]);
		entry_size = read_uint32_le(&buf[20]);

		fprintf(stdout, "extracting %s\n", filename);

		out = fopen(filename, "wb");

		if(out == NULL)
		{
			fprintf(stderr, "failed to open %s for writing: %s\n", filename, strerror(errno));
			return 1;
		}

		arc_offset = ftell(mif);
		fseek(mif, entry_offset, SEEK_SET);

		while(entry_size--)
			fputc(fgetc(mif), out);

		fseek(mif, arc_offset, SEEK_SET);
		fclose(out);
	}

	return 0;
}
