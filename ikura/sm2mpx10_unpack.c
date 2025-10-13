#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _OPENMP
#include <omp.h>
#endif

#include "readint.h"
#include "xprintf.h"

#define BUFFER_SIZE 8192

struct sm2_entry
{
	unsigned long offset;
	unsigned long size;
	char filename[13];
};

int main(int argc, char *argv[])
{
	int i;
	FILE *arc, *lst;
	unsigned char buf[32];
	unsigned int count, index_size;
	unsigned char magic[] = {"\x53\x4d\x32\x4d\x50\x58\x31\x30"};

	if(argc < 2)
		err_fprintf(stderr, "missing arguments\n");

	arc = fopen(argv[1], "rb");

	if(arc == NULL)
		err_fprintf(stderr, "could not open file %s\n", argv[1]);

	fread(buf, 1, 32, arc);

	if(memcmp(magic, buf, 8))
		err_fprintf(stderr, "%s is not an Ikura archive\n", argv[1]);

	count = read_uint32_le(&buf[8]);
	index_size = read_uint32_le(&buf[12]);

	/* :-) */
	memset(buf, '\0', 32);
	strcpy((char *)buf, argv[1]);
	strcat((char *)buf, ".txt");

	lst = fopen((char *)buf, "wb");

	struct sm2_entry *entries = malloc(count * sizeof(struct sm2_entry));

	if(entries == NULL)
		err_fprintf(stderr, "Out of memory\n");

	/* TODO: handle parsing errors */
	for(i = 0; i < count; i++)
	{
		fread(buf, 1, 20, arc);

		memset(entries[i].filename, 0, 13);
		memcpy(entries[i].filename, buf, 12);
		entries[i].offset = read_uint32_le(&buf[12]);
		entries[i].size = read_uint32_le(&buf[16]);

		fprintf(lst, "%s\n", entries[i].filename);
	}

	fclose(lst);
	fclose(arc);

	#pragma omp parallel for
	for(i = 0; i < count; i++)
	{
		FILE *entry, *arc_thread = fopen(argv[1], "rb");
		unsigned long entry_size, bytes_read;
		unsigned char buffer[BUFFER_SIZE];

		if(!arc_thread)
		{
			fprintf(stderr, "Failed to open %s for thread %d\n", argv[1], i);
			continue;
		}

		fprintf(stdout, "extracting %s\n", entries[i].filename);

		entry = fopen(entries[i].filename, "wb");
		if(!entry)
		{
			fprintf(stderr, "Failed to create %s\n", entries[i].filename);
			fclose(arc_thread);
			continue;
		}

		fseek(arc_thread, entries[i].offset, SEEK_SET);
		entry_size = entries[i].size;

		while(entry_size > 0)
		{
			bytes_read = fread(buffer, 1, entry_size < BUFFER_SIZE ? entry_size : BUFFER_SIZE, arc_thread);
			fwrite(buffer, 1, bytes_read, entry);
			entry_size -= bytes_read;
		}

		fclose(entry);
		fclose(arc_thread);
	}

	free(entries);
	return 0;
}
