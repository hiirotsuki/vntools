#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _OPENMP
#include <omp.h>
#endif

#include "readint.h"
#include "xprintf.h"
#include "lzss.h"

struct ifl_entry
{
	unsigned long offset;
	unsigned long size;
	char filename[17];
};

int main(int argc, char *argv[])
{
	int i;
	unsigned int count;
	FILE *ifl, *lst;
	unsigned char header_buf[12];
	unsigned char ifl_magic[] = {"\x49\x46\x4c\x53"};

	if(argc < 2)
		err_fprintf(stderr, "Usage: ifl_unpack <*.IFL>\n");

	ifl = fopen(argv[1], "rb");

	if(ifl == NULL)
		err_fprintf(stderr, "could open file %s\n", argv[1]);

	fread(header_buf, 1, 12, ifl);

	if(memcmp(header_buf, ifl_magic, 4))
		err_fprintf(stderr, "%s is not an IFL archive\n", argv[1]);

	lst = fopen("list.txt", "wb");

	/*data_offset = read_uint32_le(&header_buf[4]);*/
	count = read_uint32_le(&header_buf[8]);

	struct ifl_entry *entries = malloc(count * sizeof(struct ifl_entry));

	if(!entries)
		err_fprintf(stderr, "Out of memory\n");

	memset(entries, 0, count * sizeof(struct ifl_entry));

	for(i = 0; i < count; i++)
	{
		unsigned char buf[24];
		fread(buf, 1, 24, ifl);
		memcpy(entries[i].filename, buf, 16);
		entries[i].offset = read_uint32_le(&buf[16]);
		entries[i].size = read_uint32_le(&buf[20]);

		fprintf(lst, "%s\n", entries[i].filename);

#ifdef DEBUG
		fprintf(stdout, "filename: %s, offset: %lu, size: %lu\n", entries[i].filename, entries[i].offset, entries[i].size);
#endif
	}

	fclose(lst);
	fclose(ifl);

	#pragma omp parallel for
	for(i = 0; i < count; i++)
	{
		char *p;
		int compressed = 0;
		unsigned char buf[24];
		FILE *out, *ifl_thread = fopen(argv[1], "rb");
		unsigned long entry_size;

		if(!ifl_thread)
		{
			fprintf(stderr, "Failed to open %s for thread %d\n", argv[1], i);
			continue;
		}

		entry_size = entries[i].size;

		fseek(ifl_thread, entries[i].offset, SEEK_SET);

		p = strrchr(entries[i].filename, '.');

		if(p)
			if(!strcmp(p, ".grd"))
				compressed = 1;

		if(compressed)
		{
			char local_filename[17];
			unsigned char *compressed_data, *uncompressed_data;
			unsigned long compressed_size, uncompressed_size;

			strcpy(local_filename, entries[i].filename);
			p = strrchr(local_filename, '.');

			fprintf(stdout, "Decompressing and extracting %s\n", entries[i].filename);

			p[1] = 'b';
			p[2] = 'm';
			p[3] = 'p';

			/* TODO: ??? 12 bytes, 4 are the real size, the other 8 are...? */
			fread(buf, 1, 12, ifl_thread);
			uncompressed_size = read_uint32_le(&buf[4]);

			compressed_size = entry_size - 12;

			compressed_data = malloc(compressed_size);
			uncompressed_data = malloc(uncompressed_size);

			if(!compressed_data || !uncompressed_data)
			{
				fprintf(stderr, "Failed to allocate memory for %s\n", entries[i].filename);
				free(compressed_data);
				free(uncompressed_data);
				fclose(ifl_thread);
				continue;
			}

			fread(compressed_data, 1, compressed_size, ifl_thread);

			if(lzss_decompress(uncompressed_data, uncompressed_size, compressed_data, compressed_size) != 0)
			{
				fprintf(stderr, "Failed to decompress %s\n", entries[i].filename);
				free(compressed_data);
				free(uncompressed_data);
				fclose(ifl_thread);
				continue;
			}

			out = fopen(local_filename, "wb");
			if(out == NULL)
			{
				fprintf(stderr, "Failed to create %s\n", local_filename);
				free(compressed_data);
				free(uncompressed_data);
				fclose(ifl_thread);
				continue;
			}

			fwrite(uncompressed_data, 1, uncompressed_size, out);

			free(compressed_data);
			free(uncompressed_data);
		}
		else
		{
			out = fopen(entries[i].filename, "wb");
			if(out == NULL)
			{
				fprintf(stderr, "Failed to create %s\n", entries[i].filename);
				fclose(ifl_thread);
				continue;
			}
			fprintf(stdout, "extracting %s\n", entries[i].filename);
			while(entry_size--)
				fputc(fgetc(ifl_thread), out);
		}

		fclose(ifl_thread);
		fclose(out);
	}

	free(entries);
	return 0;
}
