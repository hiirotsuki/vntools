#ifndef LZSS_H
#define LZSS_H

unsigned char *lzss_window_init(int window_size, const char fill_byte);
void lzss_window_free(void *ptr);
int lzss_decompress(unsigned char *output, unsigned long output_size, 
                    const unsigned char *input, unsigned long input_size);

#endif /* LZSS_H */
