/*
   This file is part of Retro Graphics Toolkit

   Retro Graphics Toolkit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or any later version.

   Retro Graphics Toolkit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Retro Graphics Toolkit. If not, see <http://www.gnu.org/licenses/>.
   Copyright Sega16 (or whatever you wish to call me) (2012-2017)
*/
#pragma once
#include <FL/Fl_Int_Input.H>
#include <FL/fl_ask.H>

#include "guidefs.h"
#include "class_global.h"
extern unsigned map_scroll_pos_x, map_scroll_pos_y;
extern unsigned map_off_x, map_off_y;
extern unsigned tile_edit_offset_x, tile_edit_offset_y;
extern unsigned tile_placer_tile_offset_y;
extern unsigned tile_edit_truecolor_off_x, tile_edit_truecolor_off_y;
extern unsigned true_color_box_x, true_color_box_y;
extern unsigned SpriteOff[2];
extern bool show_grid_placer;
extern unsigned tile_zoom_edit;
extern uint8_t truecolor_temp[4];
extern std::string the_file;//TODO refactor
extern unsigned mode_editor;/*!<Variable is used to determine which "mode" the user is in for example palette editing or map editing*/
extern bool show_grid;
extern bool G_hflip[2];
extern bool G_vflip[2];
extern bool G_highlow_p[2];
extern bool showTrueColor;
extern bool rowSolo;
extern bool tileEditModePlace_G;
extern uint32_t selTileE_G[2];
#define messageWrap(format,...) \
	if(window) \
		fl_message(format, ##__VA_ARGS__); \
	else \
		printf(format, ##__VA_ARGS__);

#define alertWrap(format,...) \
	if(window) \
		fl_alert(format, ## __VA_ARGS__); \
	else \
		fprintf(stderr, format, ##__VA_ARGS__);

void mkProgress(Fl_Window**winP, Fl_Progress**progress);
int SafeTxtInput(Fl_Int_Input*in, bool redraw = true);
int SafeTxtInputZeroAllowed(Fl_Int_Input*in, bool redraw = true);
void updateTileSelectAmt(void);
void updateTileSelectAmt(uint32_t newMax);
int menuPopupVector(const char * title, const char * text, std::vector<std::string>&slst);
int MenuPopup(const char * title, const char * text, unsigned num, unsigned def, ...);
int menuPopupArray(const char * title, const char * text, unsigned def, const Fl_Menu_Item*arr);
bool loadOrSaveFile(std::string&result, const char * the_tile = "Pick a file", bool save_file = false);
bool verify_str_number_only(const char * str);
