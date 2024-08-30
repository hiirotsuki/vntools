#ifndef CP932_H
#define CP932_H

int cp932_to_utf8(unsigned char *cp932, int cp932_len, char **utf8);
int utf16_to_utf8(char *s, unsigned int *r);

#endif
