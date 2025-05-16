#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void err_fprintf(FILE *stream, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stream, format, args);
	va_end(args);

	fflush(stream);

	exit(EXIT_FAILURE);
}
