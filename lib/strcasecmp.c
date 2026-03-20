#include "strcasecmp.h"

#ifndef HAVE_STRCASECMP

static int vn_tolower(int c)
{
	return c >= 'A' && c <= 'Z' ? c + 32 : c;
}

int strcasecmp(const char *s1, const char *s2)
{
	const unsigned char *c1, *c2;
	c1 = (unsigned char *)s1; c2 = (unsigned char *)s2;
	for(; *c1 && *c2 && (*c1 == *c2 || vn_tolower(*c1) == vn_tolower(*c2)); c1++, c2++);
	return vn_tolower(*c1) - vn_tolower(*c2);
}

#endif
