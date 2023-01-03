#include <stdio.h>
#include <string.h>

#include "readbytes.h"

int main(int argc, char *argv[])
{
	char *p;
	long pos;
	FILE *ifl, *out;
	char filename[17];
	unsigned int count;
	unsigned char buf[24];
	unsigned char ifl_magic[] = {"\x49\x46\x4c\x53"};
	unsigned long entry_offset, entry_size, entry_uncomp_size;

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

	fread(buf, 1, 12, ifl);

	if(memcmp(buf, ifl_magic, 4))
	{
		fprintf(stderr, "%s is not an IFL archive\n", argv[1]);
		return 1;
	}

	/*data_offset = read_uint32_le(&buf[4]);*/
	count = read_uint32_le(&buf[8]);

	while(count--)
	{
		memset(filename, '\0', 17);
		fread(buf, 1, 24, ifl);
		memcpy(filename, buf, 16);

		entry_offset = read_uint32_le(&buf[16]);
		entry_size = read_uint32_le(&buf[20]);

		pos = ftell(ifl);

		fseek(ifl, entry_offset, SEEK_SET);

		p = strrchr(filename, '.');
		if(p)
		{
			if(!strcmp(p, ".grd"))
			{
				unsigned char window[4096] = {0x20};
				unsigned int lzss_pos = 4078;
				int cbit, control, match_pos, match_len, done = 0;

				fprintf(stdout, "decompressing and extracting %s\n", filename);

				p[1] = 'b';
				p[2] = 'm';
				p[3] = 'p';

				out = fopen(filename, "wb");

				fread(buf, 1, 12, ifl);
				entry_uncomp_size = read_uint32_le(&buf[4]);

				while(1)
				{
					if(ftell(out) == (long)entry_uncomp_size || done == 1)
						break;

					control = fgetc(ifl);
					for(cbit = 0x01; cbit & 0xff; cbit <<= 1)
					{
						if(done)
							break;
						if(control & cbit)
						{
							fputc((window[lzss_pos++] = (unsigned char)fgetc(ifl)), out);
							lzss_pos &= 4095;
						}
						else
						{
							match_pos = fgetc(ifl);
							match_len = fgetc(ifl);
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
				out = fopen(filename, "wb");
				fprintf(stdout, "extracting %s\n", filename);
				while(entry_size--)
					fputc(fgetc(ifl), out);
			}
		}
		else
		{
			out = fopen(filename, "wb");
			fprintf(stdout, "extracting %s\n", filename);
			while(entry_size--)
				fputc(fgetc(ifl), out);
		}
		fseek(ifl, pos, SEEK_SET);
		fclose(out);
	}

	fclose(ifl);
	return 0;
}
