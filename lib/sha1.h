#ifndef SHA1_H
#define SHA1_H

#include <stdint.h>
#include <string.h>

typedef struct
{
	uint32_t h[5];
	uint32_t Nl, Nh;
	uint32_t data[16];
	unsigned int num;
} SHA_CTX;

#define SHA_DIGEST_LENGTH 20

int SHA1_Init(SHA_CTX *ctx);
int SHA1_Update(SHA_CTX *ctx, const void *data, size_t len);
int SHA1_Final(unsigned char *md, SHA_CTX *ctx);

unsigned char *SHA1(const unsigned char *d, size_t n, unsigned char *md);

#endif /* SHA1_H */
