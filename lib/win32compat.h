#ifndef WIN32COMPAT_H
#define WIN32COMPAT_H

#ifdef _WIN32
extern FILE *win32_fopen(const char *path, const char *mode);
extern int win32_mkdir(const char *path);
extern char *win32_strchr(const char *s, int c);
#define fopen(path, mode) win32_fopen(path, mode)
#define mkdir(path, mode) win32_mkdir(path)
#define strchr(str, ch) win32_strchr(str, ch)
#endif

#endif /* !WIN32COMPAT_H */
