#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "readint.h"
#include "xmkdir.h"
#include "cp932.h"
#include "win32compat.h"

int main(int argc, char *argv[])
{
	char *filename;
	int is_packed = 0;
	unsigned int count;
	unsigned char buf[128];
	const char *magic = "ARCX";
	long offset, size, unpacked_size, index;
	FILE *arcx, *out;

	if(argc < 2)
		return 1;

	arcx = fopen(argv[1], "rb");

	/* ARCX, magic identifier */
	/* count, 32-bit integer */
	/* unknown, 32-bit integer */
	/* data offset, 32-bit integer */
	if(fread(buf, 1, 16, arcx) != 16)
	{
		fprintf(stdout, "unexpected read issue\n");
		return 1;
	}

	if(memcmp(buf, magic, 4))
	{
		fprintf(stdout, "not an ARCX archive\n");
		return 1;
	}

	count = read_uint32_le(&buf[4]);

	fprintf(stdout, "extracting %d files\n", count);
	while(count--)
	{
		fread(buf, 128, 1, arcx);
		if(cp932_to_utf8((char *)buf, &filename) < 0)
			return 1; /* todo: proper error handling, refactor into caller malloc? */
		offset = read_uint32_le(&buf[100]);
		size = read_uint32_le(&buf[104]);
		unpacked_size = read_uint32_le(&buf[108]);

		is_packed = (size != unpacked_size) ? 1 : 0;

		fprintf(stdout, "extracting %s\n", filename);

		/* todo: error handling */
		recursive_mkdir(filename, 0755, '\\');

		out = fopen(filename, "wb");
		index = ftell(arcx);
		fseek(arcx, offset, SEEK_SET);

		if(!is_packed)
		{
			while(size--)
				fputc(fgetc(arcx), out);
		}
		else
		{
			unsigned int lzss_pos = 4078;
			unsigned char window[4096] = {0};
			int control, cbit, match_pos, match_len;

			while(1)
			{
				if(ftell(out) == unpacked_size)
					goto done;

				control = fgetc(arcx);
				for(cbit = 0x01; cbit & 0xFF; cbit <<= 1)
				{
					if(control & cbit)
					{
						fputc((window[lzss_pos++] = fgetc(arcx)), out);
						lzss_pos &= 4095;
					}
					else
					{
						match_pos = fgetc(arcx);
						match_len = fgetc(arcx);
						match_pos |= (match_len & 0xF0) << 4;
						match_len = (match_len & 0x0F) + 3;
						while(match_len--)
						{
							if(ftell(out) == unpacked_size)
								goto done;

							fputc((window[lzss_pos++] = window[match_pos++]), out);
							lzss_pos &= 4095;
							match_pos &= 4095;
						}
					}
				}
			}
		}
		done:
		fseek(arcx, index, SEEK_SET);
		fclose(out);
		free(filename);
	}

	fclose(arcx);
	return 0;
}
