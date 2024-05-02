#ifndef INCLUDE_READBYTES_H
#define INCLUDE_READBYTES_H

#include <stdint.h>
#include <string.h>

uint32_t read_uint32_be(const unsigned char *bytes)
{
	return(((uint32_t)bytes[3]) | ((uint32_t)bytes[2] << 8) | ((uint32_t)bytes[1] << 16) | ((uint32_t)bytes[0] << 24));
}

uint32_t read_uint32_le(const unsigned char *bytes)
{
	return(((uint32_t)bytes[0]) | ((uint32_t)bytes[1] << 8) | ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24));
}

uint16_t read_uint16_be(const unsigned char *bytes)
{
	return(((uint16_t)bytes[0] << 8) | ((uint16_t)bytes[1]));
}

uint16_t read_uint16_le(const unsigned char *bytes)
{
	return(((uint16_t)bytes[0]) | ((uint16_t)bytes[1] << 8));
}

#ifdef NEED_64BIT_PARSING
uint64_t read_uint64_le(const unsigned char *bytes)
{
	return read_uint32_le(&bytes[0]) | read_uint32_le(&bytes[4]) << 8;
}
#endif
#endif
