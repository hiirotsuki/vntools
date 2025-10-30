#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef _OPENMP
#include <omp.h>
#endif

#include "readint.h"
#include "xmkdir.h"
#include "cp932.h"
#include "normalize.h"
#include "lzss.h"
#include "xprintf.h"
#include "xread.h"
#ifdef _WIN32
#include "win32compat.h"
#endif

struct arcx_entry
{
	long offset;
	long size;
	long unpacked_size;
	int is_packed;
	char *filename;
};

int main(int argc, char *argv[])
{
	unsigned int count, i;
	unsigned char buf[128];
	const char *magic = "ARCX";
	FILE *arcx;
	struct arcx_entry *entries;

	if(argc < 2)
		err_fprintf(stderr, "Usage: arcx_unpack <*.ARCX>\n");

	arcx = fopen(argv[1], "rb");
	if(!arcx)
		err_fprintf(stderr, "could not open file: %s\n", argv[1]);

	/* ARCX, magic identifier */
	/* count, 32-bit integer */
	/* unknown, 32-bit integer */
	/* data offset, 32-bit integer */
	if(fread(buf, 1, 16, arcx) != 16)
		err_fprintf(stderr, "unexpected read issue\n");

	if(memcmp(buf, magic, 4))
		err_fprintf(stderr, "not an ARCX archive\n");

	count = read_uint32_le(&buf[4]);

	entries = malloc(count * sizeof(struct arcx_entry));
	if(!entries)
		err_fprintf(stderr, "Out of memory\n");

	fprintf(stdout, "extracting %d files\n", count);

	for(i = 0; i < count; i++)
	{
		int namelen;
		memset(buf, '\0', 128);
		if(fread(buf, 128, 1, arcx) != 1)
		{
			fprintf(stderr, "failed to read entry %d\n", i);
			for(unsigned int j = 0; j < i; j++)
				free(entries[j].filename);
			free(entries);
			fclose(arcx);
			return 1;
		}

		namelen = strlen((char *)buf);
		entries[i].filename = malloc((namelen * 4) + 1);
		if(!entries[i].filename)
		{
			fprintf(stderr, "out of memory\n");
			for(unsigned int j = 0; j < i; j++)
				free(entries[j].filename);
			free(entries);
			fclose(arcx);
			return 1;
		}

		if(cp932_to_utf8(buf, namelen, &entries[i].filename) < 0)
		{
			fprintf(stderr, "filename encoding error\n");
			for(unsigned int j = 0; j <= i; j++)
				free(entries[j].filename);
			free(entries);
			fclose(arcx);
			return 1;
		}

		entries[i].offset = read_uint32_le(&buf[100]);
		entries[i].size = read_uint32_le(&buf[104]);
		entries[i].unpacked_size = read_uint32_le(&buf[108]);
		entries[i].is_packed = (entries[i].size != entries[i].unpacked_size) ? 1 : 0;
	}

	fclose(arcx);

	#pragma omp parallel for
	for(i = 0; i < count; i++)
	{
		FILE *arcx_thread, *out;
		unsigned char buffer[BUFFER_SIZE];
		unsigned long bytes_read, remaining;
		unsigned char *compressed_data, *uncompressed_data;

		arcx_thread = fopen(argv[1], "rb");
		if(!arcx_thread)
		{
			fprintf(stderr, "Failed to open %s for thread %d\n", argv[1], i);
			continue;
		}

		fprintf(stdout, "extracting %s\n", entries[i].filename);

		recursive_mkdir(entries[i].filename, 0755);

		out = fopen(entries[i].filename, "wb");
		if(!out)
		{
			fprintf(stderr, "Failed to create %s\n", entries[i].filename);
			fclose(arcx_thread);
			continue;
		}

		fseek(arcx_thread, entries[i].offset, SEEK_SET);

		if(!entries[i].is_packed)
		{
			remaining = entries[i].size;
			while(remaining > 0)
			{
				bytes_read = xread(buffer, remaining, arcx_thread);
				fwrite(buffer, 1, bytes_read, out);
				remaining -= bytes_read;
			}
		}
		else
		{
			compressed_data = malloc(entries[i].size);
			uncompressed_data = malloc(entries[i].unpacked_size);

			if(!compressed_data || !uncompressed_data)
			{
				fprintf(stderr, "Failed to allocate memory for %s\n", entries[i].filename);
				free(compressed_data);
				free(uncompressed_data);
				fclose(out);
				fclose(arcx_thread);
				continue;
			}

			if(fread(compressed_data, 1, entries[i].size, arcx_thread) != entries[i].size)
			{
				fprintf(stderr, "Failed to read compressed data for %s\n", entries[i].filename);
				free(compressed_data);
				free(uncompressed_data);
				fclose(out);
				fclose(arcx_thread);
				continue;
			}

			if(lzss_decompress(uncompressed_data, entries[i].unpacked_size, compressed_data, entries[i].size) != 0)
			{
				fprintf(stderr, "Failed to decompress %s\n", entries[i].filename);
				free(compressed_data);
				free(uncompressed_data);
				fclose(out);
				fclose(arcx_thread);
				continue;
			}

			fwrite(uncompressed_data, 1, entries[i].unpacked_size, out);

			free(compressed_data);
			free(uncompressed_data);
		}

		fclose(out);
		fclose(arcx_thread);
	}

	for(i = 0; i < count; i++)
		free(entries[i].filename);
	free(entries);

	return 0;
}
