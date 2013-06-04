/*! \file global.h
Header for globals included with all other files.
*/ 
#pragma once
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)
#include "includes.h"
#include "class_global.h"
#include "class_palette.h"
#include "errorMsg.h"
#include "class_tiles.h"
//functions
/*!
This function calculates pointer offset for truecolor tiles
for example cal_offset_truecolor(8,8,0,32);
would edit the red pixel (8,8) located on tile 32 
*/
void tileToTrueCol(uint8_t * input,uint8_t * output,uint8_t row);
bool saveBinAsText(void * ptr,size_t sizeBin,FILE * myfile);
bool verify_str_number_only(char * str);
uint32_t cal_offset_truecolor(uint16_t x,uint16_t y,uint16_t rgb,uint32_t tile);
//void draw_tile(uint16_t x_off,uint16_t y_off,uint32_t tile_draw,uint8_t zoom,uint8_t pal_row,bool hflip_tile,bool vflip_tile);
//void draw_truecolor(uint32_t tile,uint16_t x,uint16_t y,bool usehflip,bool usevflip,uint8_t zoom);
bool load_file_generic(const char * the_tile="Pick a file",bool save_file=false);
//bool load_file_generic(const char * the_tile,bool save_file);
void truecolor_to_tile(uint8_t palette_row,uint32_t tile);
uint8_t find_near_color_from_row(uint8_t row,uint8_t r,uint8_t g,uint8_t b);
uint8_t find_near_color_from_row_rgb(uint8_t row,uint8_t r,uint8_t g,uint8_t b);
//map related functions
void resize_tile_map(uint16_t new_x,uint16_t new_y);
void set_tile_full(uint32_t tile,uint16_t x,uint16_t y,uint8_t palette_row,bool use_hflip,bool use_vflip,bool highorlow_prio);
void set_tile(uint32_t tile,uint16_t x,uint16_t y);
void set_prio(uint16_t x,uint16_t y,bool prio_set);
void set_hflip(uint16_t x,uint16_t y,bool hflip_set);
void set_vflip(uint16_t x,uint16_t y,bool vflip_set);
void set_palette_type(uint8_t type);
uint32_t MakeRGBcolor(uint32_t pixel,float saturation = 1.1f, float hue_tweak = 0.0f,float contrast = 1.0f, float brightness = 1.0f,float gamma = 2.2f);
//uint32_t MakeRGBcolor(uint32_t pixel,float saturation, float hue_tweak,float contrast, float brightness ,float gamma );
//varibles and defines
#define sega_genesis 0
#define NES 1
extern uint8_t game_system;/*!< sets which game system is in use*/
extern Fl_Group * shadow_highlight_switch;
extern bool shadow_highlight;
extern uint16_t map_scroll_pos_x;
extern uint16_t map_scroll_pos_y;
//tabs group id
extern  intptr_t pal_id;
extern  intptr_t tile_edit_id;
extern  intptr_t tile_place_id;
extern bool show_grid_placer;
extern uint8_t tile_zoom_edit;
extern uint8_t truecolor_temp[4];
//extern uint8_t * truecolor_tiles;
extern std::string the_file;//this is for tempory use only
extern uint8_t mode_editor;/*!< Importan varible is used to determin which "mode" the user is in for example palette editing or map editing*/
#define pal_edit 0
#define tile_edit 1
#define tile_place 2
#define default_map_off_x 304
extern uint16_t map_off_x,map_off_y;
#define default_map_off_y 232
#define tile_placer_tile_offset_x 120
#define default_tile_placer_tile_offset_y 208
#define tile_place_buttons_x_off 16
extern uint16_t tile_placer_tile_offset_y;
#define palette_bar_offset_x 16
#define default_palette_bar_offset_y 56
#define palette_preview_box_x 408
#define palette_preview_box_y 208
//extern uint8_t palette_bar_offset_y;
#define default_tile_edit_offset_x 344
//#define tile_edit_offset_x 472
#define default_tile_edit_offset_y 224
extern uint16_t tile_edit_offset_y;
extern uint16_t tile_edit_offset_x;
#define default_tile_edit_truecolor_off_x 8;
#define default_tile_edit_truecolor_off_y 224;
extern uint16_t tile_edit_truecolor_off_x,tile_edit_truecolor_off_y;
#define true_color_box_size 48
#define default_true_color_box_y 112
#define default_true_color_box_x 320
extern uint16_t true_color_box_x,true_color_box_y;
extern bool show_grid;
//extern uint8_t palette_muliplier;
extern bool G_hflip;
extern bool G_vflip;
extern bool G_highlow_p;
//extern uint8_t palette_adder;
extern uint32_t file_size;
extern uint8_t ditherAlg;
extern uint8_t palTypeGen;
extern const uint8_t palTab[];
extern bool showTrueColor;
