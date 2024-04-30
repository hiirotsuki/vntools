#include <stdio.h>
#include <string.h>
#include <errno.h>
#ifndef _WIN32
#include <sys/stat.h>
#endif

#include "win32compat.h"

int recursive_mkdir(char *filename, int mode, char sep)
{
	char *p;
#ifdef _WIN32
	(void)mode;
#endif

	if(strchr(filename, sep))
	{
		for(p = filename; *p; p++)
		{
			if(*p == sep)
			{
				*p = '\0';

				if(mkdir(filename, mode))
				{
					if(errno != EEXIST)
						return 1;
				}
				*p = sep;
			}
		}
	}
	return 0;
}
