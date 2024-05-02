#ifndef WIN32COMPAT_H
#define WIN32COMPAT_H

#ifdef _WIN32
extern FILE *win32_fopen(const char *path, const char *mode);
extern int win32_mkdir(const char *path);
#define fopen(path, mode) win32_fopen(path, mode)
#define mkdir(path, mode) win32_mkdir(path)
#endif

#endif /* !WIN32COMPAT_H */
