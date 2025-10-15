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

void lzss_window_free(void *ptr)
{
	if(ptr)
		free(ptr);
	ptr = NULL;
}
