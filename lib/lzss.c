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

#define WINDOW_SIZE 4096
#define START_POSITION 4078
#define LOOKAHEAD_SIZE 18
#define MIN_MATCH_LENGTH 3

unsigned long lzss_compress_bound(unsigned long input_size)
{
	return input_size + (input_size / 8) + 18;
}

int lzss_compress(unsigned char *output, unsigned long output_size, const unsigned char *input, unsigned long input_size)
{
	unsigned char *window;
	int i, j;
	int window_pos = START_POSITION;
	unsigned char code_buf[17];
	int code_buf_ptr;
	unsigned int code_buf_mask;
	unsigned long input_pos = 0;
	unsigned long output_pos = 0;

	window = lzss_window_init(WINDOW_SIZE, 0x20);
	if(!window)
		return -1;

	code_buf[0] = 0;
	code_buf_ptr = 1;
	code_buf_mask = 1;

	while(input_pos < input_size)
	{
		int best_len = 0;
		int best_pos = 0;
		unsigned char ahead[LOOKAHEAD_SIZE];
		int ahead_len = 0;

		for(i = 0; i < LOOKAHEAD_SIZE && (input_pos + i) < input_size; i++)
		{
			ahead[i] = input[input_pos + i];
			ahead_len++;
		}

		if(ahead_len == 0)
			break;

		if(ahead_len >= MIN_MATCH_LENGTH)
		{
			for(i = 0; i < WINDOW_SIZE; i++)
			{
				int len = 0;
				int match_pos = i;
				int test_window_pos = window_pos;
				unsigned char saved[LOOKAHEAD_SIZE];
				int k;

				/* Save window positions we'll modify */
				for(k = 0; k < ahead_len; k++)
					saved[k] = window[(test_window_pos + k) % WINDOW_SIZE];

				for(j = 0; j < ahead_len; j++)
				{
					unsigned char window_byte = window[match_pos];
					if(window_byte != ahead[j])
						break;

					window[test_window_pos] = window_byte;
					match_pos = (match_pos + 1) % WINDOW_SIZE;
					test_window_pos = (test_window_pos + 1) % WINDOW_SIZE;
					len++;
				}

				/* Restore window */
				for(k = 0; k < len; k++)
					window[(window_pos + k) % WINDOW_SIZE] = saved[k];

				if(len >= MIN_MATCH_LENGTH && len > best_len)
				{
					best_len = len;
					best_pos = i;
				}
			}
		}

		if(best_len >= MIN_MATCH_LENGTH)
		{
			code_buf[0] &= ~code_buf_mask;
			code_buf[code_buf_ptr++] = (unsigned char)best_pos;
			code_buf[code_buf_ptr++] = (unsigned char)(((best_pos >> 8) << 4) | (best_len - MIN_MATCH_LENGTH));

			for(i = 0; i < best_len; i++)
			{
				if(input_pos >= input_size)
					break;

				window[window_pos] = input[input_pos++];
				window_pos = (window_pos + 1) % WINDOW_SIZE;
			}
		}
		else
		{
			/* Output literal */
			if(input_pos >= input_size)
				break;

			code_buf[0] |= code_buf_mask;
			code_buf[code_buf_ptr++] = input[input_pos];

			window[window_pos] = input[input_pos++];
			window_pos = (window_pos + 1) % WINDOW_SIZE;
		}

		code_buf_mask <<= 1;

		if(code_buf_mask == 256)
		{
			for(i = 0; i < code_buf_ptr; i++)
			{
				if(output_pos >= output_size)
				{
					lzss_window_free(window);
					return -1;
				}
				output[output_pos++] = code_buf[i];
			}
			code_buf[0] = 0;
			code_buf_ptr = 1;
			code_buf_mask = 1;
		}
	}

	if(code_buf_ptr > 1)
	{
		for(i = 0; i < code_buf_ptr; i++)
		{
			if(output_pos >= output_size)
			{
				lzss_window_free(window);
				return -1;
			}
			output[output_pos++] = code_buf[i];
		}
	}

	lzss_window_free(window);
	return output_pos;
}
