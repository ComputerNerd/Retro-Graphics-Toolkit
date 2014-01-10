//here is dithering function for images
#pragma once
#include <inttypes.h>
void ditherImage(uint8_t * image,uint16_t w,uint16_t h,bool useAlpha=false,bool colSpace=false,bool forceRow=false,uint8_t forcedrow=0);
