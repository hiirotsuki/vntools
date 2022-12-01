#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "readbytes.h"
#include "cp932.h"

#ifdef _WIN32
#include "win32compat.h"
#endif

int main(int argc, char **argv)
{
	int f;
	long pos;
	char *filename;
	FILE *archive_file, *out_file;
	unsigned char buf[76], magic[] = {"\x50\x41\x43\x4b"};
	unsigned int index_count, compressed, entry_size, entry_orig_size, entry_offset;

	if(argc < 3)
	{
		fprintf(stderr, "nekopunch_unpack: you must specify -name32 or -name64 and archive\nExample: nekopunch_unpack -name32 test.pak\n");
		return 1;
	}

	if(strcmp(argv[1], "-name32") && strcmp(argv[1], "-name64"))
	{
		fprintf(stderr, "invalid argument");
		return 1;
	}

	if(!strcmp(argv[1], "-name32"))
		f = 32;
	else if(!strcmp(argv[1], "-name64"))
		f = 64;

	archive_file = fopen(argv[2], "rb");

	if(!archive_file)
	{
		fprintf(stderr, "could not open archive %s\n", argv[2]);
		return 1;
	}

	fread(buf, 1, 4, archive_file);

	if(memcmp(magic, buf, 4))
	{
		fprintf(stderr, "not a studio neko punch archive");
		return 1;
	}

	fread(buf, 1, 12, archive_file);
	index_count = read_uint32_le(buf);
	compressed  = read_uint32_le(&buf[4]);

	while(index_count)
	{
		if(f == 32)
		{
			fread(buf, 1, 44, archive_file);
			entry_orig_size = read_uint32_le(&buf[32]);
			entry_size      = read_uint32_le(&buf[36]);
			entry_offset    = read_uint32_le(&buf[40]);
		}
		else if(f == 64)
		{
			fread(buf, 1, 76, archive_file);
			entry_orig_size = read_uint32_le(&buf[64]);
			entry_size      = read_uint32_le(&buf[68]);
			entry_offset    = read_uint32_le(&buf[72]);
		}

		if(cp932_to_utf8((char *)buf, &filename) == -2)
		{
			fprintf(stderr, "Out of memory");
			fclose(archive_file);
			return 1;
		}
		out_file = fopen(filename, "wb");

		pos = ftell(archive_file);
		fseek(archive_file, entry_offset, SEEK_SET);

		printf("extracting %s\n", filename);

		if(!compressed)
		{
			while(entry_size--)
				fputc(fgetc(archive_file), out_file);
		}
		else
		{
			unsigned int lzss_pos = 4078;
			unsigned char window[4096] = {0};
			int control, cbit, match_pos, match_len;

			while(1)
			{
				if(ftell(out_file) == entry_orig_size)
					break;

				control = fgetc(archive_file);
				for(cbit = 0x01; cbit & 0xff; cbit <<= 1)
				{
					if(control & cbit)
					{
						fputc((window[lzss_pos++] = fgetc(archive_file)), out_file);
						lzss_pos &= 4095;
					}
					else
					{
						match_pos = fgetc(archive_file);
						match_len = fgetc(archive_file);
						match_pos |= (match_len & 0xf0) << 4;
						match_len = (match_len & 0x0f) + 3;
						while(match_len--)
						{
							if(ftell(out_file) == entry_orig_size)
								break;
							fputc((window[lzss_pos++] = window[match_pos++]), out_file);
							lzss_pos &= 4095;
							match_pos &= 4095;
						}
					}
				}
			}
		}

		fseek(archive_file, pos, SEEK_SET);
		fclose(out_file);
		free(filename);
		index_count--;
	}

	fclose(archive_file);
	return 0;
}
