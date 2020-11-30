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
	Copyright Sega16 (or whatever you wish to call me) (2012-2018)
*/
#pragma once
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Window.H>

#include <inttypes.h>
#include <string>
#include <utility>
#include <vector>

#include "project.h"
#include "system.h"
extern int pushed_g;
#define TABS_WITH_ROW_BUTTONS 2
class editor : public Fl_Double_Window {
private:
	Fl_Menu_Bar *menu;
	void _editor();
	void draw_non_gui();
	unsigned tilesSpriteOff[2];
public:
	std::vector<std::pair<unsigned, int64_t>>*keepUserdat;
	std::vector<std::pair<std::string, int64_t>>*luaCallback;
	void updateChunkSel(void);
	void updateMapWH(uint32_t w, uint32_t h);
	void updateMapWH(void);
	void updateTileMapGUI(uint32_t x, uint32_t y);
	void updateBlockTilesChunk(uint32_t prj);
	void updateBlockTilesChunk(void);
	void updateSpriteSliders(uint32_t prj);
	void updateSpriteSliders(void);
	void updateChunkGUI(uint32_t tx, uint32_t ty);
	void updateChunkSize(uint32_t wi, uint32_t hi);
	void updateChunkSize(void);
	bool tabsHidden[shareAmtPj];
	Fl_Box* cordDisp[2];//The purpose of this is to display coordinates of tile in plane and chunk editor
	Fl_Box*totalTiles;
	Fl_Choice*subSysC;
	Fl_Choice*solidChunkMenu;
	Fl_Choice*ditherAlgSel;
	Fl_Choice*planeSelect;
	Fl_Choice*gameSysSel;
	Fl_Choice*luaScriptSel;
	Fl_Input*luaScriptName;
	std::vector<Fl_Group*>tabsMain;
	Fl_Scrollbar * map_x_scroll;
	Fl_Scrollbar * map_y_scroll;
	Fl_Scrollbar* chunkX;
	Fl_Scrollbar* chunkY;
	Fl_Int_Input* map_w;
	Fl_Int_Input* map_h;
	Fl_Int_Input* chunksize[2];//chunk width,height
	Fl_Int_Input* map_amt;
	Fl_Int_Input* tmapOffset;
	Fl_Slider*planeSelectChunk;
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
	Fl_Slider* spriteselgroup;
	Fl_Slider* spritesel;
	Fl_Slider* spriteslat;
	Fl_Int_Input* spritesoff[2];
	Fl_Slider* spritest;//sprite start tile
	Fl_Slider* spritesize[2];//sprite width,height
	Fl_Slider* spritepalrow;//selects palette row
	Fl_Slider* spritezoom;
	Fl_Slider*metaspritesel;
	Fl_Input*curPlaneName;
	Fl_Input* spritegrouptxt;
	Fl_Button*spritealign[4];
	Fl_Input* spriteglobaltxt;
	Fl_Input*spritemetatxt;
	Fl_Slider* projectSelect;
	Fl_Tabs* the_tabs;
	Fl_Round_Button* palType[12];
	Fl_Round_Button* palRTE[MAX_ROWS_PALETTE * TABS_WITH_ROW_BUTTONS];
	Fl_Check_Button* hflipCB[2];
	Fl_Check_Button* vflipCB[2];
	Fl_Check_Button* prioCB[2];
	Fl_Check_Button* sharePrj[shareAmtPj];
	Fl_Check_Button* BlocksCBtn;
	Fl_Check_Button* spritehflip;
	Fl_Check_Button* spritevflip;
	Fl_Check_Button* spriteprio;
	Fl_Check_Button* useBlocksChunkCBtn;
	Fl_Slider* shareWith[shareAmtPj];
	Fl_Check_Button* havePrj[shareAmtPj];
	Fl_Text_Buffer* TxtBufProject;
	Fl_Text_Buffer* luaBufProject;
	Fl_Text_Editor* TxtEditProject;
	Fl_Text_Editor* luaEditProject;
	void draw();
	editor(int X, int Y, int W, int H, const char *L = 0);
	editor(int W, int H, const char *L = 0);
	int handle(int);
	unsigned getCurrentTileCurrentTab()const;
};
extern editor *window;
