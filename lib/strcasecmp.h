#ifndef LIB_STRCASECMP_H
#define LIB_STRCASECMP_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if HAVE_STRCASECMP
#include <strings.h>
#else
int strcasecmp(const char *s1, const char *s2);
#endif

#endif
