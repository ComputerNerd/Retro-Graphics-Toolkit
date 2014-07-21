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
#include "project.h"
#include <inttypes.h>
class editor : public Fl_Double_Window{
	private:
	Fl_Menu_Bar *menu;
	void _editor();
	void draw_non_gui();
	public:
	bool tabsHidden[shareAmtPj+1];//The purpose of the +1 is to accomidate for the settings/project tab
	Fl_Box* cordDisp[2];//The purpose of this is to display coordinates of tile in plane and chunk editor
	Fl_Choice* subSysC;
	Fl_Choice* solidChunkMenu;
	Fl_Group* TabsMain[shareAmtPj+1];
	Fl_Scrollbar * map_x_scroll;
	Fl_Scrollbar * map_y_scroll;
	Fl_Scrollbar* chunkX;
	Fl_Scrollbar* chunkY;
	Fl_Slider* map_w;
	Fl_Slider* map_h;
	Fl_Slider* map_amt;
	Fl_Slider* rgb_red;
	Fl_Slider* rgb_green;
	Fl_Slider* rgb_blue;
	Fl_Slider* rgb_alpha;
	Fl_Slider* ditherPower;
	Fl_Slider* pal_size;
	Fl_Slider* tile_size;
	Fl_Slider* place_tile_size;
	Fl_Slider* chunk_tile_size;
	Fl_Slider* tile_select;
	Fl_Slider* tile_select_2;
	Fl_Slider* chunk_select;
	Fl_Slider* tile_select_3;
	Fl_Slider* projectSelect;
	Fl_Tabs* the_tabs;
	Fl_Round_Button* palType[9];
	Fl_Round_Button* GameSys[2];
	Fl_Round_Button* palRTE[4];
	Fl_Check_Button* hflipCB[2];
	Fl_Check_Button* vflipCB[2];
	Fl_Check_Button* prioCB[2];
	Fl_Check_Button* sharePrj[shareAmtPj];
	Fl_Check_Button* BlocksCBtn;
	Fl_Check_Button* useBlocksChunkCBtn;
	Fl_Slider* shareWith[shareAmtPj];
	Fl_Check_Button* havePrj[shareAmtPj];
	Fl_Text_Buffer* TxtBufProject;
	Fl_Text_Editor* TxtEditProject;
	void draw();
	editor(int X, int Y, int W, int H, const char *L = 0);
	editor(int W, int H, const char *L = 0);
	int handle(int);
};
extern editor *window;
