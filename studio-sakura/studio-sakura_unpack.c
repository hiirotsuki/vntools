#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "readint.h"
#include "cp932.h"

#ifdef _WIN32
#include "win32compat.h"
#endif

int main(int argc, char *argv[])
{
	long pos;
	char *c, *filename;
	unsigned char buf[256];
	FILE *arc_file, *out_file;
	unsigned int count, entry_size, entry_offset;

	if(argc < 2)
	{
		fprintf(stderr, "missing arguments\n");
		return 1;
	}

	arc_file = fopen(argv[1], "rb");

	if(!arc_file)
	{
		fprintf(stderr, "could not open file %s\n", argv[1]);
		return 1;
	}

	fread(buf, 1, 4, arc_file);
	fseek(arc_file, 32, SEEK_SET);
	count = read_uint32_le(buf);

	while(count)
	{
		int namelen;
		fread(buf, 1, 256, arc_file);

		namelen = strlen((char *)buf);
		filename = malloc((namelen * 4) + 1);

		if(!filename)
		{
			fprintf(stderr, "Out of memory\n");
			return 1;
		}

		cp932_to_utf8(buf, namelen, &filename);

		/* strip compression file extension */
		c = strrchr(filename, '.');
		if(c)
			if(!strcmp(c, ".pr3"))
				*c = '\0';

		fprintf(stdout, "extracting %s\n", filename);

		fread(buf, 1, 16, arc_file);

		entry_size = read_uint32_le(&buf[0]);
		entry_offset = read_uint32_le(&buf[4]);

		pos = ftell(arc_file);
		out_file = fopen(filename, "wb");

		fseek(arc_file, entry_offset, SEEK_SET);

		/* are uncompressed files supported? */
		fread(buf, 1, 36, arc_file);

		if(!memcmp(buf, "ACMPRS03", 8))
		{
			unsigned int lzss_pos = 4078;
			unsigned char window[4096] = {0};
			int control, cbit, match_pos, match_len;
#if 0
			entry_comp_size = read_uint32_le(&buf[20]);
#endif
			while(1)
			{
				if(ftell(out_file) >= entry_size)
					break;
				control = fgetc(arc_file);
				for(cbit = 0x01; cbit & 0xff; cbit <<= 1)
				{
					if(control & cbit)
					{
						fputc((window[lzss_pos++] = fgetc(arc_file)), out_file);
						lzss_pos &= 4095;
					}
					else
					{
						match_pos = fgetc(arc_file);
						match_len = fgetc(arc_file);
						match_pos |= (match_len & 0xf0) << 4;
						match_len = (match_len & 0x0f) + 3;
						while(match_len--)
						{
							if(ftell(out_file) >= entry_size)
								break;
							fputc((window[lzss_pos++] = window[match_pos++]), out_file);
							lzss_pos &= 4095;
							match_pos &= 4095;
						}
					}
				}
			}
		}
		else
		{
			fprintf(stderr, "uncompressed file, not implemented\n");
			break;
		}

		fseek(arc_file, pos, SEEK_SET);
		fclose(out_file);
		free(filename);
		count--;
	}

	fclose(arc_file);
	return 0;
}
