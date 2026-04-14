#define _CRT_SECURE_NO_WARNINGS
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

struct opk_entry
{
	unsigned long offset;
	unsigned long size;
	char filename[13]; /* 8 + .ogg + null */
};

int unpack_archive(const char *archive_path, const char *list_path, int print_file_list);
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
		return unpack_archive(argv[2], argc >= 4 ? argv[3] : NULL, 0);
	}
	else if(strcmp(argv[1], "-l") == 0 || strcmp(argv[1], "--list") == 0)
	{
		if(argc < 3)
		{
			fprintf(stderr, "Error: list mode requires an archive file\n");
			print_usage(argv[0]);
			return 1;
		}
		return unpack_archive(argv[2], NULL, 1);
	}
	else if(strcmp(argv[1], "-p") == 0 || strcmp(argv[1], "--pack") == 0)
	{
		if(argc < 4)
		{
			fprintf(stderr, "Error: pack mode requires an archive name and a file list\n");
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
	fprintf(stderr, "OPK Archive Tool - Pack and Unpack Cromwell audio archives\n\n");
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "  Unpack: %s [-u|--unpack] <archive.opk> [list.txt]\n", progname);
	fprintf(stderr, "  List:   %s [-l|--list]   <archive.opk>\n", progname);
	fprintf(stderr, "  Pack:   %s [-p|--pack]   <archive.opk> <list.txt>\n\n", progname);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -u, --unpack    Extract all files from an archive\n");
	fprintf(stderr, "  -p, --pack      Create an archive from a file list\n");
	fprintf(stderr, "  -l, --list      List the files in an archive\n");
	fprintf(stderr, "  -h, --help      Show this help message\n");
}

int unpack_archive(const char *archive_path, const char *list_path, int print_file_list)
{
	int i;
	FILE *opk, *lst;
	unsigned char buf[20];
	long count, name_pos, index_pos;
	const char *magic = "VoiceOggPackFile";
	unsigned int entry_offset, entry_next;
	char list_buf[256];
	struct opk_entry *entries;

	opk = fopen(archive_path, "rb");

	if(opk == NULL)
		err_fprintf(stderr, "could not open file %s\n", archive_path);

	if(fread(buf, 1, 20, opk) != 20)
		err_fprintf(stderr, "unexpected end of archive\n");

	if(memcmp(magic, buf, 16))
		err_fprintf(stderr, "%s is not a cromwell audio archive\n", archive_path);

	count = read_uint32_le(&buf[16]);

	fprintf(stdout, "Archive contains %ld %s\n", count, count == 1 ? "file" : "files");

	entries = malloc(count * sizeof(struct opk_entry));

	if(entries == NULL)
		err_fprintf(stderr, "Out of memory\n");

	memset(entries, 0, count * sizeof(struct opk_entry));

	fseek(opk, -(count * 8), SEEK_END);
	name_pos = ftell(opk);

	fseek(opk, 20, SEEK_SET);
	fread(buf, 1, 4, opk);
	entry_next = read_uint32_le(buf);
	index_pos = ftell(opk);

	for(i = 0; i < count; i++)
	{
		fseek(opk, name_pos, SEEK_SET);
		fread(entries[i].filename, 1, 8, opk);
		strcat(entries[i].filename, ".ogg");
		name_pos += 8;

		fseek(opk, index_pos, SEEK_SET);
		entry_offset = entry_next;
		fread(buf, 1, 4, opk);
		entry_next = read_uint32_le(buf);

		entries[i].offset = entry_offset;
		entries[i].size = entry_next - entry_offset;

		index_pos = ftell(opk);
	}

	fclose(opk);

	if(print_file_list)
	{
		for(i = 0; i < count; i++)
			fprintf(stdout, "%s (%lu bytes)\n", entries[i].filename, entries[i].size);

		free(entries);
		return 0;
	}

	if(list_path)
	{
		lst = fopen(list_path, "wb");
	}
	else
	{
		snprintf(list_buf, sizeof(list_buf), "%s.txt", archive_path);
		lst = fopen(list_buf, "wb");
	}

	if(lst == NULL)
		err_fprintf(stderr, "could not create file list\n");

	for(i = 0; i < count; i++)
		fprintf(lst, "%s\n", entries[i].filename);

	fclose(lst);

	#pragma omp parallel for
	for(i = 0; i < count; i++)
	{
		FILE *opk_thread = fopen(archive_path, "rb");
		FILE *ogg;
		unsigned long entry_size, bytes_read;
		unsigned char buffer[BUFFER_SIZE];

		if(opk_thread == NULL)
		{
			fprintf(stderr, "Failed to open %s for thread %d\n", archive_path, i);
			continue;
		}

		fprintf(stdout, "  extracting %s (%lu bytes)\n", entries[i].filename, entries[i].size);

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

	fprintf(stdout, "File list saved to: %s\n", list_path ? list_path : list_buf);
	fprintf(stdout, "Unpacking complete!\n");

	return 0;
}

int pack_archive(const char *archive_path, const char *list_path)
{
	char *p;
	int count, i;
	char filename[13];
	FILE *opk, *ogg, *lst;
	const char *magic = "VoiceOggPackFile";
	long name_offset, data_offset, ogg_size, l;

	fprintf(stdout, "Packing archive: %s\n", archive_path);

	lst = fopen(list_path, "rb");
	if(lst == NULL)
	{
		fprintf(stderr, "Error: could not open file list %s\n", list_path);
		return 1;
	}

	count = 0;
	while(1)
	{
		memset(filename, '\0', 13);
		if(!fgets(filename, 13, lst))
			break;
		count++;
	}
	rewind(lst);

	fprintf(stdout, "Packing %d files\n", count);

	opk = fopen(archive_path, "wb");
	if(opk == NULL)
	{
		fprintf(stderr, "Error: could not create archive %s\n", archive_path);
		fclose(lst);
		return 1;
	}

	fputs(magic, opk);
	fwrite_uint32_le(count, opk);

	data_offset = (count * 4) + 24;
	fwrite_uint32_le(data_offset, opk);

	for(i = 0; i <= count; i++)
	{
		memset(filename, '\0', 13);
		if(!fgets(filename, 13, lst))
			break;

		if((p = strrchr(filename, '\r')))
			*p = '\0';
		if((p = strrchr(filename, '\n')))
			*p = '\0';

		ogg = fopen(filename, "rb");
		if(!ogg)
		{
			fprintf(stderr, "Error: could not open %s\n", filename);
			fclose(lst);
			fclose(opk);
			return 1;
		}

		fprintf(stdout, "  writing %s\n", filename);

		fseek(ogg, 0, SEEK_END);
		ogg_size = ftell(ogg);
		rewind(ogg);

		fseek(opk, (i * 4) + 20, SEEK_SET);
		fwrite_uint32_le(data_offset, opk);

		fseek(opk, data_offset, SEEK_SET);
		for(l = 0; l < ogg_size; l++)
			fputc(fgetc(ogg), opk);

		data_offset += ogg_size;

		fclose(ogg);
	}

	fseek(opk, 0, SEEK_END);
	name_offset = ftell(opk);
	fseek(opk, (i * 4) + 20, SEEK_SET);
	fwrite_uint32_le(name_offset, opk);

	rewind(lst);
	fseek(opk, 0, SEEK_END);
	while(count--)
	{
		char buf[9];
		memset(buf, '\0', 9);
		memset(filename, '\0', 13);
		if(!fgets(filename, 13, lst))
			break;

		if((p = strrchr(filename, '.')))
			*p = '\0';

		strcpy(buf, filename);
		fwrite(buf, 1, 8, opk);
	}

	fclose(lst);
	fclose(opk);

	fprintf(stdout, "Packing complete!\n");

	return 0;
}
