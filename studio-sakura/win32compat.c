#ifdef _WIN32

#include <windows.h>
#include <direct.h>
#include <stdio.h>

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

	return _wmkdir(wpath);
}
#else
#ifdef __GNUC__
__attribute__((unused))
#endif
static int dummy = 0;
#endif
