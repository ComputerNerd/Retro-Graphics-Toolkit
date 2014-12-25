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
#include <FL/Fl_Int_Input.H>
#include "includes.h"
#define default_map_off_x 256
#define default_map_off_y 256
#define defaultspritex 192
#define defaultspritey 72
#define tile_placer_tile_offset_x 88
#define default_tile_placer_tile_offset_y 208
#define tile_place_buttons_x_off 8
#define palette_bar_offset_x 16
#define default_palette_bar_offset_y 56
#define palette_preview_box_x 408
#define palette_preview_box_y 208
#define default_tile_edit_offset_x 344
#define default_tile_edit_truecolor_off_x 8
#define default_tile_edit_truecolor_off_y 224
#define default_tile_edit_offset_y 224
#define true_color_box_size 48
#define default_true_color_box_y 188
#define default_true_color_box_x 732
#define DefaultChunkX 208
#define DefaultChunkY 80

extern unsigned map_scroll_pos_x,map_scroll_pos_y;
extern unsigned map_off_x,map_off_y;
extern unsigned tile_edit_offset_x,tile_edit_offset_y;
extern unsigned tile_placer_tile_offset_y;
extern unsigned tile_edit_truecolor_off_x,tile_edit_truecolor_off_y;
extern unsigned true_color_box_x,true_color_box_y;
extern unsigned SpriteOff[2];
//These values below must correspond with array offsets for tabs
#define pal_edit 0
#define tile_edit 1
#define tile_place 2
#define chunkEditor 3
#define spriteEditor 4
#define levelEditor 5
#define settingsTab 6

void mkProgress(Fl_Window**winP,Fl_Progress**progress);
int SafeTxtInput(Fl_Int_Input*in,bool redraw=true);
int SafeTxtInputZeroAllowed(Fl_Int_Input*in,bool redraw=true);
void updateTileSelectAmt(void);
void updateTileSelectAmt(uint32_t newMax);
int MenuPopup(const char * title,const char * text,unsigned num,...);
bool load_file_generic(const char * the_tile="Pick a file",bool save_file=false);
bool loadsavefile(std::string&fn,const char * the_tile="Pick a file",bool save_file=false);
