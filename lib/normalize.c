#include <string.h>

#include "normalize.h"

int path_normalize(char *path)
{
	int i = 0;
	char *p = path;
	for(; *p; p++)
		if(*p == '\\' || *p == '/')
		{
			i++;
			*p = SEP;
		}
	return i;
}
