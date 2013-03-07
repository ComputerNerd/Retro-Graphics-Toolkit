/*Color conversion functions should go here*/
#pragma once
unsigned char to_nes_color_rgb(unsigned char red,unsigned char green,unsigned char blue);
unsigned char to_nes_color(unsigned char pal_index);
unsigned short to_sega_genesis_color(unsigned char pal_index);
uint32_t count_colors(uint8_t * image_ptr,uint32_t w,uint32_t h,uint8_t *colors_found,bool useAlpha=false);
void update_emphesis(Fl_Widget*,void*);
