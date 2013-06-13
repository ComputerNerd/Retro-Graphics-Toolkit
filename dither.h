//here is dithering function for images
#pragma once
#include <inttypes.h>
#define plus_truncate_uchar(a, b) \
    if (((int)(a)) + (b) < 0) \
        (a) = 0; \
    else if (((int)(a)) + (b) > 255) \
        (a) = 255; \
    else \
        (a) += (b);
void ditherImage(uint8_t * image,uint16_t w,uint16_t h,bool useAlpha=false,bool colSpace=false,bool forceRow=false,uint8_t forcedrow=0);
