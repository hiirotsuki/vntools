#ifdef _WIN32

#include <windows.h>
#include <direct.h>
#include <stdio.h>
#include <errno.h>

#include "win32compat.h"

FILE *win32_fopen(const char *path, const char *mode)
{
	wchar_t wpath[32767];
	wchar_t wmode[32];
	MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, 32767);
	MultiByteToWideChar(CP_UTF8, 0, mode, -1, wmode, 32);

	return _wfopen(wpath, wmode);
}

int win32_mkdir(const char *path)
{
	wchar_t wpath[32767];
	MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, 32767);

	if(!CreateDirectoryW(wpath, NULL))
	{
		DWORD err = GetLastError();
		if(err == ERROR_ALREADY_EXISTS)
			errno = EEXIST;
		else if(err == ERROR_PATH_NOT_FOUND)
			errno = ENOENT;
		return -1;
	}

	return 0;
}

#else
#ifdef __GNUC__
__attribute__((unused))
#endif
static int dummy = 0;
#endif
