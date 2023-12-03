#ifndef INCLUDE_WRITEINT_H
#define INCLUDE_WRITEINT_H

#include <stdint.h>
#include <stdio.h>

int fwrite_int32_le(int32_t in, FILE *out)
{
	unsigned char buf[4];
	buf[0] = (int32_t)(in & 0x000000ff);
	buf[1] = (int32_t)(in & 0x0000ff00) >> 8;
	buf[2] = (int32_t)(in & 0x00ff0000) >> 16;
	buf[3] = (int32_t)(in & 0xff000000) >> 24;

	return fwrite(buf, 1, 4, out);
}

int fwrite_uint32_le(uint32_t in, FILE *out)
{
	unsigned char buf[4];
	buf[0] = (uint32_t)(in & 0x000000ff);
	buf[1] = (uint32_t)(in & 0x0000ff00) >> 8;
	buf[2] = (uint32_t)(in & 0x00ff0000) >> 16;
	buf[3] = (uint32_t)(in & 0xff000000) >> 24;

	return fwrite(buf, 1, 4, out);
}

int fwrite_uint32_be(uint32_t in, FILE *out)
{
	unsigned char buf[4];
	buf[0] = (uint32_t)(in & 0xff000000) >> 24;
	buf[1] = (uint32_t)(in & 0x00ff0000) >> 16;
	buf[2] = (uint32_t)(in & 0x0000ff00) >> 8;
	buf[3] = (uint32_t)(in & 0x000000ff);

	return fwrite(buf, 1, 4, out);
}

#endif
