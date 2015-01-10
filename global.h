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
   Copyright Sega16 (or whatever you wish to call me) (2012-2014)
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
void tileToTrueCol(uint8_t * input,uint8_t * output,uint8_t row,bool useAlpha=true,bool alphaZero=false);
bool verify_str_number_only(char * str);
uint32_t cal_offset_truecolor(unsigned x,unsigned y,unsigned rgb,uint32_t tile);
uint8_t find_near_color_from_row(unsigned row,uint8_t r,uint8_t g,uint8_t b,bool alt);
uint8_t find_near_color_from_row_rgb(unsigned row,uint8_t r,uint8_t g,uint8_t b,bool alt);
extern bool show_grid_placer;
extern unsigned tile_zoom_edit;
extern uint8_t truecolor_temp[4];
extern std::string the_file;//TODO refractor
extern unsigned mode_editor;/*!<Variable is used to determine which "mode" the user is in for example palette editing or map editing*/
extern bool show_grid;
extern bool G_hflip[2];
extern bool G_vflip[2];
extern bool G_highlow_p[2];
extern bool showTrueColor;
extern bool rowSolo;
extern bool tileEditModePlace_G;
extern uint32_t selTileE_G[2];

