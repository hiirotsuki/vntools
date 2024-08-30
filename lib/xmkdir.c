#include <stdio.h>
#include <string.h>
#include <errno.h>
#ifndef _WIN32
#include <sys/stat.h>
#endif

#include "normalize.h"
#include "win32compat.h"

int recursive_mkdir(char *filename, int mode)
{
	char *p;
#ifdef _WIN32
	(void)mode;
#endif

	path_normalize(filename);

	if(strchr(filename, SEP))
	{
		for(p = filename; *p; p++)
		{
			if(*p == SEP)
			{
				*p = '\0';

				if(mkdir(filename, mode))
				{
					if(errno != EEXIST)
						return 1;
				}
				*p = SEP;
			}
		}
	}
	return 0;
}
