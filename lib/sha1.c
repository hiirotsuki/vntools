#include "sha1.h"
#include "readint.h"

#include <stdlib.h>

/* SHA-1 constants */
#define K0 0x5A827999UL
#define K1 0x6ED9EBA1UL
#define K2 0x8F1BBCDCUL
#define K3 0xCA62C1D6UL

/* SHA-1 initial hash values */
#define H0 0x67452301UL
#define H1 0xEFCDAB89UL
#define H2 0x98BADCFEUL
#define H3 0x10325476UL
#define H4 0xC3D2E1F0UL

#define ROTL(a, b) (((a) << (b)) | ((a) >> (32 - (b))))

#define F(x, y, z) (((x) & (y)) | (~(x) & (z)))
#define G(x, y, z) ((x) ^ (y) ^ (z))
#define H(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define I(x, y, z) ((x) ^ (y) ^ (z))

static uint32_t read_be32(const unsigned char *p)
{
	return read_uint32_be(p);
}

static void write_be32(unsigned char *p, uint32_t val)
{
	p[0] = (val >> 24) & 0xff;
	p[1] = (val >> 16) & 0xff;
	p[2] = (val >> 8) & 0xff;
	p[3] = val & 0xff;
}

static void sha1_process_block(SHA_CTX *ctx, const unsigned char block[64])
{
	uint32_t W[80];
	uint32_t A, B, C, D, E, temp;
	int i;

	/* Prepare message schedule W[0..79] */
	/* W[0..15] are the 16 32-bit words of the block in big-endian */
	for (i = 0; i < 16; i++)
		W[i] = read_be32(&block[i * 4]);

	/* W[16..79] are derived from previous words */
	for (i = 16; i < 80; i++)
		W[i] = ROTL(W[i-3] ^ W[i-8] ^ W[i-14] ^ W[i-16], 1);

	A = ctx->h[0];
	B = ctx->h[1];
	C = ctx->h[2];
	D = ctx->h[3];
	E = ctx->h[4];

	for (i = 0; i < 80; i++)
	{
		if (i < 20)
			temp = ROTL(A, 5) + F(B, C, D) + E + W[i] + K0;
		else if (i < 40)
			temp = ROTL(A, 5) + G(B, C, D) + E + W[i] + K1;
		else if (i < 60)
			temp = ROTL(A, 5) + H(B, C, D) + E + W[i] + K2;
		else
			temp = ROTL(A, 5) + I(B, C, D) + E + W[i] + K3;

		E = D;
		D = C;
		C = ROTL(B, 30);
		B = A;
		A = temp;
	}

	ctx->h[0] += A;
	ctx->h[1] += B;
	ctx->h[2] += C;
	ctx->h[3] += D;
	ctx->h[4] += E;
}

int SHA1_Init(SHA_CTX *ctx)
{
	if (!ctx)
		return 0;

	ctx->h[0] = H0;
	ctx->h[1] = H1;
	ctx->h[2] = H2;
	ctx->h[3] = H3;
	ctx->h[4] = H4;

	ctx->Nl = 0;
	ctx->Nh = 0;
	ctx->num = 0;

	memset(ctx->data, 0, sizeof(ctx->data));

	return 1;
}

int SHA1_Update(SHA_CTX *ctx, const void *data, size_t len)
{
	const unsigned char *input = (const unsigned char *)data;
	size_t i;
	uint32_t l;

	if (!ctx || (!data && len > 0))
		return 0;

	l = ctx->Nl + ((uint32_t)len << 3);
	if (l < ctx->Nl) /* overflow */
		ctx->Nh++;
	ctx->Nh += (uint32_t)(len >> 29);
	ctx->Nl = l;

	for (i = 0; i < len; i++)
	{
		((unsigned char *)ctx->data)[ctx->num] = input[i];
		ctx->num++;

		if (ctx->num == 64)
		{
			sha1_process_block(ctx, (unsigned char *)ctx->data);
			ctx->num = 0;
		}
	}

	return 1;
}

int SHA1_Final(unsigned char *md, SHA_CTX *ctx)
{
	unsigned char pad[64];
	uint32_t bits_low, bits_high;
	unsigned int pad_len;
	int i;

	if (!ctx || !md)
		return 0;

	/* Save bit count */
	bits_low = ctx->Nl;
	bits_high = ctx->Nh;

	/* Pad message to 448 bits mod 512 */
	pad[0] = 0x80;
	memset(&pad[1], 0, 63);

	if (ctx->num < 56)
		pad_len = 56 - ctx->num;
	else
		pad_len = 120 - ctx->num;

	SHA1_Update(ctx, pad, pad_len);

	/* Append length in bits as 64-bit big-endian */
	write_be32(&pad[0], bits_high);
	write_be32(&pad[4], bits_low);
	SHA1_Update(ctx, pad, 8);

	/* Produce final hash value in big-endian */
	for (i = 0; i < 5; i++)
		write_be32(&md[i * 4], ctx->h[i]);

	memset(ctx, 0, sizeof(*ctx));

	return 1;
}

unsigned char *SHA1(const unsigned char *d, size_t n, unsigned char *md)
{
	static unsigned char static_md[SHA_DIGEST_LENGTH];
	SHA_CTX ctx;

	if (!md)
		md = static_md;

	SHA1_Init(&ctx);
	SHA1_Update(&ctx, d, n);
	SHA1_Final(md, &ctx);

	return md;
}
