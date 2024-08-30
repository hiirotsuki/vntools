#ifndef NORMALIZE_H
#define NORMALIZE_H

#ifdef _WIN32
#define SEP '\\'
#else
#define SEP '/'
#endif

int path_normalize(char *path);

#endif
