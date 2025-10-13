#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _OPENMP
#include <omp.h>
#endif

#include "readint.h"
#include "xprintf.h"

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
	{
		fprintf(stderr, "Usage: ifl_unpack <*.IFL>\n");
		return 1;
	}

	ifl = fopen(argv[1], "rb");

	if(!ifl)
	{
		fprintf(stderr, "could open file %s\n", argv[1]);
		return 1;
	}

	fread(header_buf, 1, 12, ifl);

	if(memcmp(header_buf, ifl_magic, 4))
	{
		fprintf(stderr, "%s is not an IFL archive\n", argv[1]);
		return 1;
	}

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
		unsigned char buf[24];
		FILE *out;
		FILE *ifl_thread = fopen(argv[1], "rb");
		unsigned long entry_size, entry_uncomp_size;

		if(!ifl_thread)
		{
			fprintf(stderr, "Failed to open %s for thread %d\n", argv[1], i);
			continue;
		}

		entry_size = entries[i].size;

		fseek(ifl_thread, entries[i].offset, SEEK_SET);

		p = strrchr(entries[i].filename, '.');
		if(p)
		{
			if(!strcmp(p, ".grd"))
			{
				char local_filename[17];
				strcpy(local_filename, entries[i].filename);
				p = strrchr(local_filename, '.');

				unsigned char window[4096] = {0x20};
				unsigned int lzss_pos = 4078;
				int cbit, control, match_pos, match_len, done = 0;

				fprintf(stdout, "Decompressing and extracting %s\n", entries[i].filename);

				p[1] = 'b';
				p[2] = 'm';
				p[3] = 'p';

				out = fopen(local_filename, "wb");
				if(!out)
				{
					fprintf(stderr, "Failed to create %s\n", local_filename);
					fclose(ifl_thread);
					continue;
				}

				fread(buf, 1, 12, ifl_thread);
				entry_uncomp_size = read_uint32_le(&buf[4]);

				while(1)
				{
					if(ftell(out) == (long)entry_uncomp_size || done == 1)
						break;

					control = fgetc(ifl_thread);
					for(cbit = 0x01; cbit & 0xff; cbit <<= 1)
					{
						if(done)
							break;
						if(control & cbit)
						{
							fputc((window[lzss_pos++] = (unsigned char)fgetc(ifl_thread)), out);
							lzss_pos &= 4095;
						}
						else
						{
							match_pos = fgetc(ifl_thread);
							match_len = fgetc(ifl_thread);
							match_pos |= (match_len & 0xf0) << 4;
							match_len = (match_len & 0x0f) + 3;
							while(match_len--)
							{
								if(ftell(out) == (long)entry_uncomp_size)
								{
									done = 1;
									break;
								}
								fputc((window[lzss_pos++] = window[match_pos++]), out);
								lzss_pos &= 4095;
								match_pos &= 4095;
							}
						}
					}
				}
			}
			else
			{
				out = fopen(entries[i].filename, "wb");
				if(!out)
				{
					fprintf(stderr, "Failed to create %s\n", entries[i].filename);
					fclose(ifl_thread);
					continue;
				}
				fprintf(stdout, "extracting %s\n", entries[i].filename);
				while(entry_size--)
					fputc(fgetc(ifl_thread), out);
			}
		}
		else
		{
			out = fopen(entries[i].filename, "wb");
			if(!out)
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
