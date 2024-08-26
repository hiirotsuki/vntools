#ifdef _WIN32

#include <windows.h>
#include <direct.h>
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>

#include "win32compat.h"

#ifndef NOFILE
FILE *win32_fopen(const char *path, const char *mode)
{
	wchar_t wpath[32767];
	wchar_t wmode[32];
	MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, 32767);
	MultiByteToWideChar(CP_UTF8, 0, mode, -1, wmode, 32);

	return _wfopen(wpath, wmode);
}
#else
void *win32_fopen(const char *path, const char *mode)
{
	int access, open_mode;

	wchar_t wpath[32767];

	MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, sizeof(wpath));
	if(!lstrcmp(mode, "rb"))
	{
		open_mode = OPEN_EXISTING;
		access = GENERIC_READ;
	}
	else if(!lstrcmp(mode, "wb"))
	{
		open_mode = CREATE_ALWAYS;
		access = GENERIC_WRITE;
	}
	else
	{
		errno = EINVAL;
		return NULL;
	}

	return CreateFileW(wpath, access, 0, NULL, open_mode, FILE_ATTRIBUTE_NORMAL, NULL);
}

int win32_fclose(void *stream)
{
	CloseHandle(stream);
	return 0;
}

int win32_fseek(FILE *stream, long offset, int whence)
{
	LARGE_INTEGER li_offset;
	li_offset.QuadPart = offset;
	SetFilePointerEx(stream, li_offset, NULL, whence);

	return 0;
}

int win32_ftell(FILE *stream)
{
	LARGE_INTEGER li_offset = {0};
	LARGE_INTEGER li_new = {0};

	SetFilePointerEx(stream, li_offset, &li_new, FILE_CURRENT);

	return li_new.QuadPart;
}

size_t win32_fread(void *ptr, size_t size, size_t nmemb, void *stream)
{
	DWORD bytes_read;
	ReadFile(stream, ptr, (size * nmemb), &bytes_read, NULL);

	/*TODO: error handling */

	return bytes_read;
}

size_t win32_fwrite(const void *ptr, size_t size, size_t nmemb, void *stream)
{
	DWORD bytes_written;
	WriteFile(stream, ptr, (size * nmemb), &bytes_written, NULL);

	/*TODO: error handling */

	return bytes_written;
}

int win32_fgetc(void *stream)
{
	unsigned char c;
	win32_fread(&c, 1, 1, stream);
	return c;
}

int win32_fputc(int character, void *stream)
{
	character = (unsigned char)character;
	win32_fwrite(&character, 1, 1, stream);
}
#endif

int win32_xvprintf(const char *format, ...)
{
	int i;
	va_list args;
	char buf[1024];
	wchar_t wbuf[2048];

	va_start(args, format);
	wvsprintfA(buf, format, args);
	va_end(args);

	i = MultiByteToWideChar(CP_UTF8, 0, buf, -1, wbuf, sizeof(wbuf));

	WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), &wbuf, i, NULL, NULL);

	return 0;
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

char *win32_strchr(const char *s, int c)
{
	c = (unsigned char)c;

	for(; *s != '\0' && *s != c; s++)
		;

	return *s == c ? (char *)s : NULL;
}

char win32_strrchr(const char *s, int c)
{
	DWORD len = lstrlen(s);
	c = (unsigned char)c;
	while(len--)
		if(s[len] == c)
			return s + len;
	return 0;
}

#else
#ifdef __GNUC__
__attribute__((unused))
#endif
static int dummy = 0;
#endif
