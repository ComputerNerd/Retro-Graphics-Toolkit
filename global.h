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
#include "errorMsg.h"
#include "class_tiles.h"
#include "palette.h"
#include "gui.h"
//functions
void tileToTrueCol(uint8_t * input,uint8_t * output,uint8_t row,bool useAlpha=true,bool alphaZero=false);
bool verify_str_number_only(char * str);
uint32_t cal_offset_truecolor(unsigned x,unsigned y,unsigned rgb,uint32_t tile);
uint8_t find_near_color_from_row(unsigned row,uint8_t r,uint8_t g,uint8_t b,bool alt);
uint8_t find_near_color_from_row_rgb(unsigned row,uint8_t r,uint8_t g,uint8_t b,bool alt);
//map related functions
uint32_t MakeRGBcolor(uint32_t pixel,float saturation = 1.2f, float hue_tweak = 0.0f,float contrast = 1.0f, float brightness = 1.0f,float gamma = 2.2f);
//varibles and defines
//tabs group id
extern bool show_grid_placer;
extern unsigned tile_zoom_edit;
extern uint8_t truecolor_temp[4];
//extern uint8_t * truecolor_tiles;
extern std::string the_file;//this is for tempory use only
extern unsigned mode_editor;/*!< Importan varible is used to determin which "mode" the user is in for example palette editing or map editing*/
extern bool show_grid;
//extern uint8_t palette_muliplier;
extern bool G_hflip[2];
extern bool G_vflip[2];
extern bool G_highlow_p[2];
//extern uint8_t palette_adder;

extern bool showTrueColor;
extern bool rowSolo;
extern bool tileEditModePlace_G;
extern uint32_t selTileE_G[2];

#define PI 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679821480865132823066470938446095505822317253594081284811174502841027019385211055596446229489549303819644288109756659334461284756482337867831652712019091456485669234603486104543266482133936072602491412737245870066063155881748815209209628292540917153643678925903600113305305488204665213841469519415116094330572703657595919530921861173819326117931051185480744623799627495673518857527248912279381830119491298336733624406566430860213949463952247371907021798609437027705392171762931767523846748184676694051320005681271452635608277857713427577896091736371787214684409012249534301465495853710507922796892589235420199561121290219608640344181598136297747713099605187072113499999983729780499510597317328160963185950244594553469083026425223082533446850352619311881710100031378387528865875332083814206171776691473035982534904287554687311595628638823537875937519577818577805321712268066130019278766111959092164201989
