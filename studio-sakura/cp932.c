#include <stdlib.h>
#include <string.h>

extern unsigned int cp932_table[];

static int cp932_to_utf16(unsigned int *r, unsigned char *s)
{
	int ret = 0;
	unsigned int buf;

	if((*s >= 0x81 && *s <= 0x9f) || (*s >= 0xe0 && *s <= 0xef) || (*s >=0xfa && *s <= 0xfc))
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

static int utf16_to_utf8(char *s, unsigned int *r)
{
	if(*r == (*r & 0x7f))
	{
		*s = *r & 0x7f;
		return 1;
	}
	else if(*r == (*r & 0x7ff))
	{
		*(s++) = 0300 | ((*r >> 6) & 0277);
		*s = 0200 | (*r&077);
		return 2;
	}
	else if(*r == (*r & 0xffff))
	{
		*(s++) = 0340 | ((*r >> 12) & 0337);
		*(s++) = 0200 | ((*r >> 6) & 077);
		*s = 0200 | (*r & 077);
		return 3;
	}
	else if(*r == (*r & 0x10ffff))
	{
		*(s++) = 0360 | ((*r >> 18) & 0357);
		*(s++) = 0200 | ((*r >> 12) & 077);
		*(s++) = 0200 | ((*r >> 6) & 077);
		*s = 0200 | (*r & 077);
		return 4;
	}
	else
	{
		return -1;
	}
}

int cp932_to_utf8(const char *cp932, char **utf8)
{
	char buf[4];
	unsigned int r;
	int slen, ulen, len, i, j;

	len = strlen(cp932);

	*utf8 = malloc((len * 4) + 1);

	if(!*utf8)
		return -2;

	i = 0;
	while(len)
	{
		slen = cp932_to_utf16(&r, (unsigned char *)cp932);

		if(slen < 0)
			return -1;

		ulen = utf16_to_utf8(buf, &r);

		cp932 += slen;
		len -= slen;

		for(j = 0; j < ulen; j++)
		{
			(*utf8)[i++] = buf[j];
		}
	}
	(*utf8)[i] = '\0';

	return 0;
}
