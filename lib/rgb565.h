#ifndef RGB565_H
#define RGB565_H

#include <stdint.h>

/*
 * RGB565: bits 15-11 R, 10-5 G, 4-0 B.
 *
 * Channel expansion to 8 bits replicates the high bits into the low bits.
 * Single-pixel RGB order: rgb[0]=R, rgb[1]=G, rgb[2]=B. Packed 0x00RRGGBB.
 */

void rgb565_to_rgb888(uint16_t rgb565, uint8_t rgb[3]);
uint32_t rgb565_to_rgb888_u32(uint16_t rgb565);

uint16_t rgb888_to_rgb565(uint8_t r, uint8_t g, uint8_t b);
uint16_t rgb888_u32_to_rgb565(uint32_t rgb);

/* Row-major top-down RGB888 (3 bytes/pixel) to matching RGB565 LE (2 bytes/pixel). */
void rgb888_raw_to_rgb565_raw(const uint8_t *rgb888, uint8_t *rgb565_le, uint32_t width, uint32_t height);

#endif
