#include <string.h>

void swap_vertical(const unsigned char *inbuf, unsigned char *outbuf, int width, int height, int stride)
{
    int j;

    outbuf += (height - 1) * stride;

    for(j = 0; j < height; j++)
    {
        memcpy(outbuf, inbuf, stride);
        inbuf += stride;
        outbuf -= stride;
    }
}
