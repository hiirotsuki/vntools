#include <stdlib.h>
#include <string.h>

unsigned char *lzss_window_init(int window_size, const char fill_byte)
{
	unsigned char *window = malloc(window_size);
	if(window == NULL)
		return NULL;
	memset(window, fill_byte, window_size);
	return window;
}

void lzss_window_reset(unsigned char *window, int window_size, const char fill_byte)
{
	memset(window, fill_byte, window_size);
}

void lzss_window_free(void *ptr)
{
	if(ptr)
		free(ptr);
	ptr = NULL;
}

int lzss_decompress(unsigned char *output, unsigned long output_size, const unsigned char *input, unsigned long input_size)
{
	unsigned char *window;
	unsigned int lzss_pos = 4078;
	unsigned long input_pos = 0;
	unsigned long output_pos = 0;
	int cbit, control, match_pos, match_len;

	window = lzss_window_init(4096, 0x20);
	if(!window)
		return -1;

	while(input_pos < input_size && output_pos < output_size)
	{
		control = input[input_pos++];

		for(cbit = 0x01; cbit & 0xff; cbit <<= 1)
		{
			if(output_pos >= output_size)
				break;

			if(input_pos >= input_size)
				break;

			if(control & cbit)
			{
				/* Literal byte */
				output[output_pos++] = window[lzss_pos++] = input[input_pos++];
				lzss_pos &= 4095;
			}
			else
			{
				if(input_pos + 1 >= input_size)
					break;

				match_pos = input[input_pos++];
				match_len = input[input_pos++];
				match_pos |= (match_len & 0xf0) << 4;
				match_len = (match_len & 0x0f) + 3;

				while(match_len--)
				{
					if(output_pos >= output_size)
						break;

					output[output_pos++] = window[lzss_pos++] = window[match_pos++];
					lzss_pos &= 4095;
					match_pos &= 4095;
				}
			}
		}
	}

	lzss_window_free(window);
	return 0;
}
