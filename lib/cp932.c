#include <stdlib.h>
#include <string.h>

extern unsigned int cp932_table[];

static int cp932_to_utf16(unsigned int *r, unsigned char *s)
{
	int ret = 0;
	unsigned int buf;

	if((*s >= 0x81 && *s <= 0x9F) || (*s >= 0xE0 && *s <= 0xEF) || (*s >=0xFA && *s <= 0xFC))
	{
		buf = *(s++) << 8;
		buf |= *s;
		ret = 2;
	}
	else
	{
		buf = *s;
		ret = 1;
	}

	*r = cp932_table[buf];

	if(!*r)
		return -1;

	return ret;
}

int utf16_to_utf8(char *s, unsigned int *r)
{
	if(*r == (*r & 0x7F))
	{
		*s = *r & 0x7F;
		return 1;
	}
	else if(*r == (*r & 0x7FF))
	{
		*(s++) = 0xC0 | ((*r >> 6) & 0xBF);
		*s = 0x80 | (*r & 0x3F);
		return 2;
	}
	else if(*r == (*r & 0xFFFF))
	{
		*(s++) = 0xE0 | ((*r >> 12) & 0xDF);
		*(s++) = 0x80 | ((*r >> 6) & 0x3F);
		*s = 0x80 | (*r & 0x3F);
		return 3;
	}
	else if(*r == (*r & 0x10FFFF))
	{
		*(s++) = 0xF0 | ((*r >> 18) & 0xEF);
		*(s++) = 0x80 | ((*r >> 12) & 0x3F);
		*(s++) = 0x80 | ((*r >> 6) & 0x3F);
		*s = 0x80 | (*r & 0x3F);
		return 4;
	}
	else
		return -1;
}

int cp932_to_utf8(unsigned char *cp932, int cp932_len, char **utf8)
{
	int wlen;
	unsigned int r;
	char *addr = *utf8;

	while(cp932_len)
	{
		wlen = cp932_to_utf16(&r, cp932);

		if(wlen < 0)
			return -1;

		*utf8 += utf16_to_utf8(*utf8, &r);

		cp932 += wlen;
		cp932_len -= wlen;
	}
	**utf8 = '\0';
	*utf8 = addr; /* restore initial position of pointer */
	return 0;
}
