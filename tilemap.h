#pragma once
#include <inttypes.h>
void sub_tile_map(uint32_t oldTile,uint32_t newTile,bool hflip=false,bool vflip=false);
bool truecolor_to_image(uint8_t * the_image,int8_t useRow=-1,bool useAlpha=true);
void generate_optimal_palette(Fl_Widget*,void * row);
void truecolorimageToTiles(uint8_t * image,int8_t rowusage,bool useAlpha=true);
