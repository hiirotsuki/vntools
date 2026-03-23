#include "rgb565.h"

void rgb565_to_rgb888(uint16_t v, uint8_t rgb[3])
{
	unsigned int r, g, b;

	r = (v >> 11) & 0x1fu;
	g = (v >> 5) & 0x3fu;
	b = v & 0x1fu;
	rgb[0] = (uint8_t)((r << 3) | (r >> 2));
	rgb[1] = (uint8_t)((g << 2) | (g >> 4));
	rgb[2] = (uint8_t)((b << 3) | (b >> 2));
}

uint32_t rgb565_to_rgb888_u32(uint16_t v)
{
	uint8_t rgb[3];

	rgb565_to_rgb888(v, rgb);
	return ((uint32_t)rgb[0] << 16) | ((uint32_t)rgb[1] << 8) | (uint32_t)rgb[2];
}

uint16_t rgb888_to_rgb565(uint8_t r, uint8_t g, uint8_t b)
{
	unsigned int r5 = (unsigned int)r >> 3;
	unsigned int g6 = (unsigned int)g >> 2;
	unsigned int b5 = (unsigned int)b >> 3;

	return (uint16_t)((r5 << 11) | (g6 << 5) | b5);
}

uint16_t rgb888_u32_to_rgb565(uint32_t rgb)
{
	uint8_t r = (uint8_t)((rgb >> 16) & 0xffu);
	uint8_t g = (uint8_t)((rgb >> 8) & 0xffu);
	uint8_t b = (uint8_t)(rgb & 0xffu);

	return rgb888_to_rgb565(r, g, b);
}

void rgb888_raw_to_rgb565_raw(const uint8_t *rgb888, uint8_t *rgb565_le, uint32_t width, uint32_t height)
{
	uint32_t y, x;

	for(y = 0; y < height; y++)
	{
		for(x = 0; x < width; x++)
		{
			size_t o3 = ((size_t)y * (size_t)width + (size_t)x) * 3u;
			size_t o2 = ((size_t)y * (size_t)width + (size_t)x) * 2u;
			uint16_t v = rgb888_to_rgb565(rgb888[o3 + 0], rgb888[o3 + 1], rgb888[o3 + 2]);

			rgb565_le[o2 + 0] = (uint8_t)(v & 0xffu);
			rgb565_le[o2 + 1] = (uint8_t)(v >> 8);
		}
	}
}
