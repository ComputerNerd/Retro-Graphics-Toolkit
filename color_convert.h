/*Color conversion functions should go here*/
#pragma once
extern uint8_t nespaltab_r[];
extern uint8_t nespaltab_g[];
extern uint8_t nespaltab_b[];
unsigned char to_nes_color_rgb(unsigned char red,unsigned char green,unsigned char blue);
unsigned char to_nes_color(unsigned char pal_index);
uint16_t to_sega_genesis_color(uint16_t pal_index);
uint32_t count_colors(uint8_t * image_ptr,uint32_t w,uint32_t h,uint8_t *colors_found,bool useAlpha=false);
void update_emphesis(Fl_Widget*,void*);
uint8_t toNesChan(uint8_t ri,uint8_t gi,uint8_t bi,uint8_t chan);
uint32_t toNesRgb(uint8_t ri,uint8_t gi,uint8_t bi);
