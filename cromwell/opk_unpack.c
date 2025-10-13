#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _OPENMP
#include <omp.h>
#endif

#include "readint.h"
#include "xprintf.h"

#define BUFFER_SIZE 8192

struct opk_entry
{
	unsigned long offset;
	unsigned long size;
	char filename[13]; /* 8 + .ogg + null */
};

int main(int argc, char *argv[])
{
	int i;
	unsigned char buf[20];
	FILE *opk, *lst;
	long count, name_pos, index_pos;
	const char *magic = "VoiceOggPackFile";
	unsigned int entry_offset, entry_next;

	if(argc < 2)
		return 1;

	if(argc >= 3)
		lst = fopen(argv[2], "wb");
	else
		lst = fopen("list.txt", "wb");

	opk = fopen(argv[1], "rb");

	if(opk == NULL)
		err_fprintf(stderr, "could not open file %s\n", argv[1]);

	fread(buf, 1, 20, opk);

	if(memcmp(magic, buf, 16))
		err_fprintf(stderr, "%s is not a cromwell audio archive\n", argv[1]);

	count = read_uint32_le(&buf[16]);

	struct opk_entry *entries = malloc(count * sizeof(struct opk_entry));

	if(entries == NULL)
		err_fprintf(stderr, "Out of memory\n");

	memset(entries, 0, count * sizeof(struct opk_entry));

	/* TODO: handle parsing errors */
	index_pos = ftell(opk);
	fseek(opk, -(count * 8), SEEK_END);
	name_pos = ftell(opk);

	fseek(opk, 20, SEEK_SET);
	fread(buf, 1, 4, opk);
	entry_next = read_uint32_le(buf);
	index_pos = ftell(opk);

	for(i = 0; i < count; i++)
	{
		/* Read filename from the name table at end of file */
		fseek(opk, name_pos, SEEK_SET);
		fread(entries[i].filename, 1, 8, opk);
		strcat(entries[i].filename, ".ogg");
		name_pos += 8;

		fprintf(lst, "%s\n", entries[i].filename);

		/* Read offset from index table */
		fseek(opk, index_pos, SEEK_SET);
		entry_offset = entry_next;
		fread(buf, 1, 4, opk);
		entry_next = read_uint32_le(buf);

		entries[i].offset = entry_offset;
		entries[i].size = entry_next - entry_offset;

		index_pos = ftell(opk);
	}

	fclose(lst);
	fclose(opk);

	#pragma omp parallel for
	for(i = 0; i < count; i++)
	{
		FILE *opk_thread = fopen(argv[1], "rb");
		FILE *ogg;
		unsigned long entry_size, bytes_read;
		unsigned char buffer[BUFFER_SIZE];

		if(opk_thread == NULL)
		{
			fprintf(stderr, "Failed to open %s for thread %d\n", argv[1], i);
			continue;
		}

		fprintf(stdout, "extracting %s\n", entries[i].filename);

		ogg = fopen(entries[i].filename, "wb");
		if(ogg == NULL)
		{
			fprintf(stderr, "Failed to create %s\n", entries[i].filename);
			fclose(opk_thread);
			continue;
		}

		fseek(opk_thread, entries[i].offset, SEEK_SET);
		entry_size = entries[i].size;

		while(entry_size > 0)
		{
			bytes_read = fread(buffer, 1, entry_size < BUFFER_SIZE ? entry_size : BUFFER_SIZE, opk_thread);
			fwrite(buffer, 1, bytes_read, ogg);
			entry_size -= bytes_read;
		}

		fclose(ogg);
		fclose(opk_thread);
	}

	free(entries);
	return 0;
}
