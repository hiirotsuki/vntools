#ifndef LZSS_H
#define LZSS_H

unsigned char *lzss_window_init(int window_size, const char fill_byte);
void lzss_window_free(void *ptr);

#endif /* LZSS_H */
