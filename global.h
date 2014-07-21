/*
 This file is part of Retro Graphics Toolkit

    Retro Graphics Toolkit is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or any later version.

    Retro Graphics Toolkit is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Retro Graphics Toolkit.  If not, see <http://www.gnu.org/licenses/>.
    Copyright Sega16 (or whatever you wish to call me (2012-2014)
*/
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
#include "palette.h"
#include "gui.h"
//functions
uint8_t nearest_color_index(uint8_t val);
void tileToTrueCol(uint8_t * input,uint8_t * output,uint8_t row,bool useAlpha=true,bool alphaZero=false);
bool verify_str_number_only(char * str);
uint32_t cal_offset_truecolor(uint16_t x,uint16_t y,uint16_t rgb,uint32_t tile);
uint8_t find_near_color_from_row(uint8_t row,uint8_t r,uint8_t g,uint8_t b);
uint8_t find_near_color_from_row_rgb(uint8_t row,uint8_t r,uint8_t g,uint8_t b);
//map related functions
uint32_t MakeRGBcolor(uint32_t pixel,float saturation = 1.1f, float hue_tweak = 0.0f,float contrast = 1.0f, float brightness = 1.0f,float gamma = 2.2f);
//uint32_t MakeRGBcolor(uint32_t pixel,float saturation, float hue_tweak,float contrast, float brightness ,float gamma );
//varibles and defines
//System declerations
#define sega_genesis 0
#define NES 1
#define frameBuffer_pal 2
#define frameBuffer 3//For example rgb565 would be here instead of frameBuffer_pal because the colors are fixed and all can be used
/*Subsystem declarations
 * Subsystem as the name implies depends on which system is selected
 * These are not compatible when switching systems
 * For the sega geneis bits 1-0 contain bit depth 0 means 1 bit
 * For the NES bit 1 contains bit depth 1 if 2 bit 0 if 1 bit
 * For palette framebuffer bits 2-0 contain bit depth add 1 to get actual just like the others
 * For framebuffer bits 4-0 contain bit depth again remember to add one to get actual bit depth
 * Valid values are
 * 0 - 1 bit black and white
 * 14 - rgb555
 * 15 - rgb565
 * 23 - rgb888
 * 31 - rgba 8888
 * Bit 4 specifies order this depends on bit depth
 * When 15 or 16`specifies byte swapping
 * When 24 bit or 32 stores as bgr or rgb
 * if bit depth is 32 bit then bit 5 is used to determin if alpha value should be before or after rgb/bgr
 * */
#define NES2x2 1//Note for version 4 or eariler projects and hence in eariler versions of Retro Graphics Toolkit bit 0 was inverted
#define NES1x1 0
#define NES2bit 2
#define NES1bit 0
extern Fl_Group * shadow_highlight_switch;
//tabs group id
extern bool show_grid_placer;
extern uint8_t tile_zoom_edit;
extern uint8_t truecolor_temp[4];
//extern uint8_t * truecolor_tiles;
extern std::string the_file;//this is for tempory use only
extern uint8_t mode_editor;/*!< Importan varible is used to determin which "mode" the user is in for example palette editing or map editing*/
extern bool show_grid;
//extern uint8_t palette_muliplier;
extern bool G_hflip[2];
extern bool G_vflip[2];
extern bool G_highlow_p[2];
//extern uint8_t palette_adder;
extern uint8_t ditherAlg;

extern bool showTrueColor;
extern bool rowSolo;
extern bool tileEditModePlace_G;
extern uint32_t selTileE_G[2];

extern uint8_t nearestAlg;
#define PI 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679821480865132823066470938446095505822317253594081284811174502841027019385211055596446229489549303819644288109756659334461284756482337867831652712019091456485669234603486104543266482133936072602491412737245870066063155881748815209209628292540917153643678925903600113305305488204665213841469519415116094330572703657595919530921861173819326117931051185480744623799627495673518857527248912279381830119491298336733624406566430860213949463952247371907021798609437027705392171762931767523846748184676694051320005681271452635608277857713427577896091736371787214684409012249534301465495853710507922796892589235420199561121290219608640344181598136297747713099605187072113499999983729780499510597317328160963185950244594553469083026425223082533446850352619311881710100031378387528865875332083814206171776691473035982534904287554687311595628638823537875937519577818577805321712268066130019278766111959092164201989
