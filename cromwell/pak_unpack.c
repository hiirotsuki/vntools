#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#ifdef _OPENMP
#include <omp.h>
#endif

#include "readint.h"
#include "xprintf.h"

struct pak_entry
{
	unsigned long offset;
	unsigned long compressed_size;
	unsigned long uncompressed_size;
	char filename[16];
};

int main(int argc, char *argv[])
{
	char *p;
	FILE *pak;
	int i, audio = 0;
	const char *magic = "Graphic PackData";
	unsigned long count, offset, next_offset;
	unsigned char buf[36], nbuf[16];

	if(argc < 2)
		return 1;

	pak = fopen(argv[1], "rb");

	if(pak == NULL)
		err_fprintf(stderr, "could not open file: %s\n", argv[1]);

	if(fread(buf, 1, 20, pak) != 20)
		err_fprintf(stderr, "could not read file %s\n", argv[1]);

	if(memcmp(buf, magic, 16))
		err_fprintf(stderr, "%s is not a cromwell pak archive\n", argv[1]);

	count = read_uint32_le(&buf[16]);

	if(strlen(argv[1]) >= 9)
		if((p = strrchr(argv[1], '.')))
			audio = (strncmp(p - 9, "VOICE", 5)) ? 1 : 0;

	struct pak_entry *entries = malloc(count * sizeof(struct pak_entry));

	if(entries == NULL)
	{
		fprintf(stderr, "Out of memory\n");
		return 1;
	}

	for(i = 0; i < count; i++)
	{
		/* filenames are not C strings */
		fread(buf, 1, 20, pak);
		memset(entries[i].filename, '\0', 16);
		memcpy(entries[i].filename, buf, 12);

		if((p = strrchr(entries[i].filename, '.')))
		{
			if(!strcmp(p, ".prs"))
				if(!audio)
					strcat(entries[i].filename, ".bmp");
				else
					strcat(entries[i].filename, ".wav");
		}

		offset = read_uint32_le(&buf[12]);
		entries[i].uncompressed_size = read_uint32_le(&buf[16]);
		entries[i].offset = offset;

		/* Determine compressed size by looking at next entry's offset */
		if(i < count - 1)
		{
			fread(nbuf, 1, 16, pak);
			next_offset = read_uint32_le(&nbuf[12]);
			fseek(pak, -16, SEEK_CUR);
		}
		else
		{
			fseek(pak, 0, SEEK_END);
			next_offset = ftell(pak);
			fseek(pak, offset, SEEK_SET);
		}

		entries[i].compressed_size = next_offset - offset;
	}

	fclose(pak);

	#pragma omp parallel for
	for(i = 0; i < count; i++)
	{
		FILE *pak_thread = fopen(argv[1], "rb");
		FILE *out;
		unsigned char *cbuf, *ucbuf;
		unsigned long uncompressed_size;

		if(pak_thread == NULL)
		{
			fprintf(stderr, "Failed to open %s for thread %d\n", argv[1], i);
			continue;
		}

		fprintf(stdout, "extracting %s\n", entries[i].filename);

		cbuf = malloc(entries[i].compressed_size);
		ucbuf = malloc(entries[i].uncompressed_size);

		if(cbuf == NULL || ucbuf == NULL)
		{
			fprintf(stderr, "Failed to allocate memory for %s\n", entries[i].filename);
			free(cbuf);
			free(ucbuf);
			fclose(pak_thread);
			continue;
		}

		fseek(pak_thread, entries[i].offset, SEEK_SET);
		fread(cbuf, 1, entries[i].compressed_size, pak_thread);
		fclose(pak_thread);

		uncompressed_size = entries[i].uncompressed_size;
		if(uncompress(ucbuf, &uncompressed_size, cbuf, entries[i].compressed_size) != Z_OK)
		{
			fprintf(stderr, "Failed to decompress %s\n", entries[i].filename);
			free(cbuf);
			free(ucbuf);
			continue;
		}

		out = fopen(entries[i].filename, "wb");
		if(out == NULL)
		{
			fprintf(stderr, "Failed to create %s\n", entries[i].filename);
			free(cbuf);
			free(ucbuf);
			continue;
		}

		fwrite(ucbuf, 1, uncompressed_size, out);

		free(cbuf);
		free(ucbuf);
		fclose(out);
	}

	free(entries);
	return 0;
}
