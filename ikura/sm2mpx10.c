#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _OPENMP
#include <omp.h>
#endif

#include "readint.h"
#include "writeint.h"
#include "xprintf.h"

#define BUFFER_SIZE 8192

struct sm2_entry
{
	unsigned long offset;
	unsigned long size;
	char filename[13];
};

int unpack_archive(const char *archive_path, int create_list, int print_file_list);
int pack_archive(const char *archive_path, const char *list_path);
void print_usage(const char *progname);

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		print_usage(argv[0]);
		return 1;
	}

	if(strcmp(argv[1], "-u") == 0 || strcmp(argv[1], "--unpack") == 0)
	{
		if(argc < 3)
		{
			fprintf(stderr, "Error: unpack mode requires an archive file\n");
			print_usage(argv[0]);
			return 1;
		}
		return unpack_archive(argv[2], 1, 0);
	}
	else if(strcmp(argv[1], "-l") == 0 || strcmp(argv[1], "--list") == 0)
	{
		if(argc < 3)
		{
			fprintf(stderr, "Error: list mode requires an archive file\n");
			print_usage(argv[0]);
			return 1;
		}
		return unpack_archive(argv[2], 0, 1);
	}
	else if(strcmp(argv[1], "-p") == 0 || strcmp(argv[1], "--pack") == 0)
	{
		if(argc < 4)
		{
			fprintf(stderr, "Error: pack mode requires archive name and file list\n");
			print_usage(argv[0]);
			return 1;
		}
		return pack_archive(argv[2], argv[3]);
	}
	else if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
	{
		print_usage(argv[0]);
		return 0;
	}

	print_usage(argv[0]);
	return 1;
}

void print_usage(const char *progname)
{
	fprintf(stderr, "SM2MPX10 Archive Tool - Pack and Unpack Ikura archives\n\n");
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "  Unpack: %s [-u|--unpack] <archive.dat>\n", progname);
	fprintf(stderr, "  List:   %s [-l|--list] <archive.dat>\n", progname);
	fprintf(stderr, "  Pack:   %s [-p|--pack] <archive.dat> <filelist.txt>\n\n", progname);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -u, --unpack    Explicitly unpack an archive\n");
	fprintf(stderr, "  -p, --pack      Explicitly pack an archive\n");
	fprintf(stderr, "  -l, --list      List the files in an archive\n");
	fprintf(stderr, "  -h, --help      Show this help message\n");
}

int unpack_archive(const char *archive_path, int create_list, int print_file_list)
{
	int i;
	FILE *arc, *lst;
	unsigned char buf[32];
	unsigned int count, index_size;
	unsigned char magic[] = {"\x53\x4d\x32\x4d\x50\x58\x31\x30"};

	arc = fopen(archive_path, "rb");

	if(arc == NULL)
		err_fprintf(stderr, "could not open file %s\n", archive_path);

	fread(buf, 1, 32, arc);

	if(memcmp(magic, buf, 8))
		err_fprintf(stderr, "%s is not an Ikura archive\n", archive_path);

	count = read_uint32_le(&buf[8]);
	index_size = read_uint32_le(&buf[12]);

	fprintf(stdout, "Archive contains %u %s\n", count, count == 1 ? "file" : "files");

	memset(buf, '\0', 32);
	strcpy((char *)buf, archive_path);
	strcat((char *)buf, ".txt");

	if(create_list)
	{
		lst = fopen((char *)buf, "wb");

		if(lst == NULL)
			err_fprintf(stderr, "could not create file list %s\n", (char *)buf);
	}

	struct sm2_entry *entries = malloc(count * sizeof(struct sm2_entry));

	if(entries == NULL)
		err_fprintf(stderr, "Out of memory\n");

	for(i = 0; i < count; i++)
	{
		fread(buf, 1, 20, arc);

		memset(entries[i].filename, 0, 13);
		memcpy(entries[i].filename, buf, 12);
		entries[i].offset = read_uint32_le(&buf[12]);
		entries[i].size = read_uint32_le(&buf[16]);

		if(create_list)
			fprintf(lst, "%s\n", entries[i].filename);
	}

	if(print_file_list)
	{
		for(i = 0; i < count; i++)
			fprintf(stdout, "%s (%lu bytes)\n", entries[i].filename, entries[i].size);

		fclose(arc);
		return 0;
	}

	if(create_list)
		fclose(lst);

	fclose(arc);

	#pragma omp parallel for
	for(i = 0; i < count; i++)
	{
		FILE *entry, *arc_thread = fopen(archive_path, "rb");
		unsigned long entry_size, bytes_read;
		unsigned char buffer[BUFFER_SIZE];

		if(!arc_thread)
		{
			fprintf(stderr, "Failed to open %s for thread %d\n", archive_path, i);
			continue;
		}

		fprintf(stdout, "  extracting %s (%lu bytes)\n", entries[i].filename, entries[i].size);

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

	fprintf(stdout, "File list saved to: %s.txt\n", archive_path);
	fprintf(stdout, "Unpacking complete!\n");

	return 0;
}

int pack_archive(const char *archive_path, const char *list_path)
{
	char *p;
	FILE *arc, *entry, *lst;
	char filename[13] = {0};
	long index_offset, data_offset;
	unsigned int count = 0, entry_size;
	unsigned char magic[] = {"\x53\x4d\x32\x4d\x50\x58\x31\x30"};

	fprintf(stdout, "Packing archive: %s\n", archive_path);

	if(strlen(archive_path) > 12)
		err_fprintf(stderr, "Error: archive name is too long, max length is 12 characters\n");

	arc = fopen(archive_path, "wb");

	if(arc == NULL)
		err_fprintf(stderr, "Error: could not create archive %s\n", archive_path);

	lst = fopen(list_path, "rb");
	if(!lst)
	{
		fprintf(stderr, "Error: could not open file list %s\n", list_path);
		fclose(arc);
		return 1;
	}

	while(1)
	{
		memset(filename, '\0', 13);
		if(!fgets(filename, 13, lst))
			break;
		count++;
	}
	rewind(lst);

	fprintf(stdout, "Packing %u files\n", count);

	fwrite(magic, 1, 8, arc);
	fwrite_uint32_le(count, arc);
	fwrite_uint32_le((count * 20) + 32, arc);
	strcpy(filename, archive_path);
	fwrite(filename, 1, 12, arc);
	fwrite_uint32_le(32, arc);

	data_offset = (count * 20) + 32;

	while(1)
	{
		unsigned char buffer[BUFFER_SIZE];
		unsigned long remaining_size, bytes_read;

		memset(filename, '\0', 13);
		if(!fgets(filename, 13, lst))
			break;

		/* strip newlines */
		if((p = strrchr(filename, '\r')))
			*p = '\0';

		if((p = strrchr(filename, '\n')))
			*p = '\0';

		entry = fopen(filename, "rb");

		if(!entry)
		{
			fprintf(stderr, "Error: could not open %s\n", filename);
			fclose(lst);
			fclose(arc);
			return 1;
		}

		fseek(entry, 0, SEEK_END);
		entry_size = ftell(entry);
		rewind(entry);

		fwrite(filename, 1, 12, arc);
		fwrite_uint32_le(data_offset, arc);
		fwrite_uint32_le(entry_size, arc);

		index_offset = ftell(arc);

		fseek(arc, data_offset, SEEK_SET);

		remaining_size = entry_size;
		while(remaining_size > 0)
		{
			bytes_read = fread(buffer, 1, remaining_size < BUFFER_SIZE ? remaining_size : BUFFER_SIZE, entry);
			fwrite(buffer, 1, bytes_read, arc);
			remaining_size -= bytes_read;
		}

		fseek(arc, index_offset, SEEK_SET);
		data_offset += entry_size;

		fprintf(stdout, "  writing %s (%u bytes)\n", filename, entry_size);

		fclose(entry);
	}

	fclose(lst);
	fclose(arc);

	fprintf(stdout, "Packing complete!\n");

	return 0;
}
