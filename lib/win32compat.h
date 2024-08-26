#ifndef WIN32COMPAT_H
#define WIN32COMPAT_H

#ifdef _WIN32
#ifndef NOFILE
#undef SEEK_SET
#undef SEEK_CUR
#undef SEEK_END
#define SEEK_SET FILE_BEGIN
#define SEEK_CUR FILE_CURRENT
#define SEEK_END FILE_END

#define FILE void /* :-) */
extern void *win32_fopen(const char *path, const char *mode);
extern size_t win32_fread(void *ptr, size_t size, size_t nmemb, void *stream);
extern size_t win32_fwrite(const void *ptr, size_t size, size_t nmemb, void *stream);
extern int win32_fclose(void *stream);
extern int win32_ftell(void *stream);
extern int win32_fseek(void *stream, long offset, int whence);
extern int win32_fputc(int character, void *stream);
extern int win32_fgetc(void *stream);
#define fread(ptr, size, nmemb, stream) win32_fread(ptr, size, nmemb, stream)
#define fwrite(ptr, size, nmemb, stream) win32_fwrite(ptr, size, nmemb, stream)
#define fclose(stream) win32_fclose(stream)
#define ftell(stream) win32_ftell(stream)
#define fseek(stream, offset, whence) win32_fseek(stream, offset, whence)
#define fputc(character, stream) win32_fputc(character, stream)
#define fgetc(stream) win32_fgetc(stream)
#else
extern FILE *win32_fopen(const char *path, const char *mode);
#endif
extern int win32_xvprintf(const char *format, ...);
extern int win32_mkdir(const char *path);
extern char *win32_strchr(const char *s, int c);
extern char *win32_strrchr(const char *s, int c);
#define fopen(path, mode) win32_fopen(path, mode)
#define mkdir(path, mode) win32_mkdir(path)
#define strchr(str, ch) win32_strchr(str, ch)
#define strrchr(str, ch) win32_strrchr(str, ch)
#define xvprintf(format, ...) win32_xvprintf(format, __VA_ARGS__)
#endif

#endif /* !WIN32COMPAT_H */
