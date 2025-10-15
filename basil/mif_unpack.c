#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#ifdef _OPENMP
#include <omp.h>
#endif

#include "readint.h"
#include "xprintf.h"

#define BUFFER_SIZE 8192

#define FILENAME_ENTRY_SIZE 17

struct mif_entry
{
	unsigned long offset;
	unsigned long size;
	char filename[FILENAME_ENTRY_SIZE];
};

int main(int argc, char *argv[])
{
	FILE *mif;
	unsigned char header_buf[24];
	int i, count;

	if(argc < 2)
		err_fprintf(stderr, "usage: mif_unpack <archive.MIF>\n");

	mif = fopen(argv[1], "rb");

	if(mif == NULL)
		err_fprintf(stderr, "failed to open %s: %s\n", argv[1], strerror(errno));

	if(fread(header_buf, 1, 8, mif) != 8)
		err_fprintf(stderr, "unexpected end of archive\n");

	if(memcmp(header_buf, "MIF\0", 4))
		err_fprintf(stderr, "expected MIF archive\n");

	count = read_uint32_le(&header_buf[4]);

	struct mif_entry *entries = malloc(count * sizeof(struct mif_entry));

	if(entries == NULL)
		err_fprintf(stderr, "Out of memory\n");

	for(i = 0; i < count; i++)
	{
		fread(header_buf, 1, 24, mif);
		memset(entries[i].filename, '\0', FILENAME_ENTRY_SIZE); /* C string? not sure, lets be conservative */
		memcpy(entries[i].filename, header_buf, 16);
		entries[i].offset = read_uint32_le(&header_buf[16]);
		entries[i].size = read_uint32_le(&header_buf[20]);
	}

	fclose(mif);

	#pragma omp parallel for
	for(i = 0; i < count; i++)
	{
		FILE *out, *mif_thread = fopen(argv[1], "rb");
		unsigned char buffer[BUFFER_SIZE];
		unsigned long bytes_read, remaining;

		if(mif_thread == NULL)
		{
			fprintf(stderr, "Failed to open %s for thread %d\n", argv[1], i);
			continue;
		}

		fprintf(stdout, "extracting %s\n", entries[i].filename);

		out = fopen(entries[i].filename, "wb");

		if(out == NULL)
		{
			fprintf(stderr, "failed to open %s for writing: %s\n", entries[i].filename, strerror(errno));
			fclose(mif_thread);
			continue;
		}

		fseek(mif_thread, entries[i].offset, SEEK_SET);

		remaining = entries[i].size;
		while(remaining > 0)
		{
			bytes_read = fread(buffer, 1, remaining < BUFFER_SIZE ? remaining : BUFFER_SIZE, mif_thread);
			fwrite(buffer, 1, bytes_read, out);
			remaining -= bytes_read;
		}

		fclose(mif_thread);
		fclose(out);
	}

	free(entries);
	return 0;
}
