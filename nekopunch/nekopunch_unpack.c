#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _OPENMP
#include <omp.h>
#endif

#include "readint.h"
#include "xprintf.h"
#include "lzss.h"
#include "cp932.h"
#include "xread.h"

#ifdef _WIN32
#include "win32compat.h"
#endif

struct nekopunch_entry
{
	unsigned int offset;
	unsigned int size;
	unsigned int orig_size;
	char *filename;
};

int main(int argc, char **argv)
{
	int i, f;
	FILE *archive_file;
	unsigned char buf[76], magic[] = {"\x50\x41\x43\x4b"};
	unsigned int index_count, compressed;
	struct nekopunch_entry *entries;

	if(argc < 3)
		err_fprintf(stderr, "nekopunch_unpack: you must specify -name32 or -name64 and archive\nExample: nekopunch_unpack -name32 test.pak\n");

	if(strcmp(argv[1], "-name32") && strcmp(argv[1], "-name64"))
		err_fprintf(stderr, "invalid argument\n");

	if(!strcmp(argv[1], "-name32"))
		f = 32;
	else if(!strcmp(argv[1], "-name64"))
		f = 64;

	archive_file = fopen(argv[2], "rb");

	if(archive_file == NULL)
		err_fprintf(stderr, "could not open archive %s\n", argv[2]);

	fread(buf, 1, 4, archive_file);

	if(memcmp(magic, buf, 4))
		err_fprintf(stderr, "not a studio neko punch archive\n");

	fread(buf, 1, 12, archive_file);
	index_count = read_uint32_le(buf);
	compressed  = read_uint32_le(&buf[4]);

	entries = malloc(index_count * sizeof(struct nekopunch_entry));

	if(entries == NULL)
		err_fprintf(stderr, "Out of memory\n");

	memset(entries, 0, index_count * sizeof(struct nekopunch_entry));

	/* Phase 1: Read all entries */
	for(i = 0; i < index_count; i++)
	{
		int namelen;

		if(f == 32)
		{
			fread(buf, 1, 44, archive_file);
			entries[i].orig_size = read_uint32_le(&buf[32]);
			entries[i].size      = read_uint32_le(&buf[36]);
			entries[i].offset    = read_uint32_le(&buf[40]);
		}
		else if(f == 64)
		{
			fread(buf, 1, 76, archive_file);
			entries[i].orig_size = read_uint32_le(&buf[64]);
			entries[i].size      = read_uint32_le(&buf[68]);
			entries[i].offset    = read_uint32_le(&buf[72]);
		}

		namelen = strlen((char *)buf);
		entries[i].filename = malloc((namelen * 4) + 1);
		if(entries[i].filename == NULL)
		{
			fclose(archive_file);
			err_fprintf(stderr, "Out of memory\n");
		}

		if(cp932_to_utf8(buf, namelen, &entries[i].filename) < 0)
		{
			fclose(archive_file);
			err_fprintf(stderr, "Filename encoding error\n");
		}
	}

	fclose(archive_file);

	/* Phase 2: Extract all entries in parallel */
	#pragma omp parallel for
	for(i = 0; i < index_count; i++)
	{
		FILE *arc_thread, *out_file;
		unsigned int entry_size, entry_orig_size;

		arc_thread = fopen(argv[2], "rb");

		if(arc_thread == NULL)
		{
			fprintf(stderr, "Failed to open %s for thread %d\n", argv[2], i);
			continue;
		}

		entry_size = entries[i].size;
		entry_orig_size = entries[i].orig_size;

		fseek(arc_thread, entries[i].offset, SEEK_SET);

		out_file = fopen(entries[i].filename, "wb");

		if(out_file == NULL)
		{
			fprintf(stderr, "Failed to create %s\n", entries[i].filename);
			fclose(arc_thread);
			continue;
		}

		if(!compressed)
		{
			size_t bytes_read;
			unsigned char buffer[BUFFER_SIZE];

			fprintf(stdout, "extracting %s\n", entries[i].filename);
			while(entry_size > 0)
			{
				bytes_read = xread(buffer, entry_size, arc_thread);
				fwrite(buffer, 1, bytes_read, out_file);
				entry_size -= bytes_read;
			}
		}
		else
		{
			unsigned char *data, *compressed_data, *uncompressed_data;
			size_t total_size = entry_size + entry_orig_size;

			fprintf(stdout, "Decompressing and extracting %s\n", entries[i].filename);

			data = malloc(total_size);

			if(!data)
			{
				fprintf(stderr, "Failed to allocate memory for %s\n", entries[i].filename);
				fclose(out_file);
				fclose(arc_thread);
				continue;
			}

			compressed_data = data;
			uncompressed_data = data + entry_size;

			fread(compressed_data, 1, entry_size, arc_thread);

			if(lzss_decompress(uncompressed_data, entry_orig_size, compressed_data, entry_size) != 0)
			{
				fprintf(stderr, "Failed to decompress %s\n", entries[i].filename);
				free(data);
				fclose(out_file);
				fclose(arc_thread);
				continue;
			}

			fwrite(uncompressed_data, 1, entry_orig_size, out_file);

			free(data);
		}

		fclose(out_file);
		fclose(arc_thread);
	}

	/* Cleanup */
	for(i = 0; i < index_count; i++)
		free(entries[i].filename);

	free(entries);
	return 0;
}
