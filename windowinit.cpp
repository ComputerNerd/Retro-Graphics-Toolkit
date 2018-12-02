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
#include <cmath>//Mingw workaround
#include "callbacks_palette.h"
#include "callback_tiles.h"
#include "tiles_io.h"
#include "callback_tilemap.h"
#include "callback_gui.h"
#include "callback_project.h"
#include "callback_chunk.h"
#include "callbacksprites.h"
#include "undo.h"
#include "gamedef.h"
#include "runlua.h"
#include "classpalettebar.h"
#include "callbacktilemaps.h"
#include "gui.h"
#include "project.h"
#include "class_global.h"
#include "luaconfig.h"
#include "callbacklua.h"
void set_mode_tabs(Fl_Widget*, void*) {
	Fl_Group * val = (Fl_Group*)(Fl_Tabs*)window->the_tabs->value();
	uint32_t oldTab = mode_editor;

	for (unsigned i = 0; i < window->tabsMain.size(); ++i) {
		if (val == window->tabsMain[i])
			mode_editor = i;
	}

	switch (mode_editor) {
	case pal_edit:
		palBar.updateSlider(0);
		break;

	case tile_edit:
		currentProject->tileC->current_tile = window->tile_select->value();
		palBar.updateSlider(1);
		break;

	case tile_place:
		currentProject->tileC->current_tile = window->tile_select_2->value();
		palBar.updateSlider(2);
		break;

	case spriteEditor:
		palBar.updateSlider(3);
		break;
	}

	lua_getglobal(Lconf, "switchTab");
	lua_pushinteger(Lconf, oldTab);
	lua_pushinteger(Lconf, mode_editor);
	runLuaFunc(Lconf, 2, 0);
}
static const char * freeDes = "This sets the currently selected palette entry to free meaning that this color can be changed";
static const char * lockedDes = "This sets the currently selected palette entry to locked meaning that this color cannot be changed but tiles can still use it";
static const char * reservedDes = "This sets the currently selected palette entry to reserved meaning that this color cannot be changed or used in tiles note that you may need make sure all tiles get re-dithered to ensure that this rule is enforced";
static const Fl_Menu_Item ditherChoices[] = {
	{"Floyd Steinberg", 0, set_ditherAlg, (void *)0},
	{"Riemersma", 0, set_ditherAlg, (void *)1},
	{"Nearest Color", 0, set_ditherAlg, (void *)2},
	{"Vertical dithering", 0, set_ditherAlg, (void *)3},
	{"Yliluoma 1", 0, set_ditherAlg, (void *)4},
	{"Yliluoma 2", 0, set_ditherAlg, (void *)5},
	{"Yliluoma 3", 0, set_ditherAlg, (void *)6},
	{"Thomas Knoll", 0, set_ditherAlg, (void *)7},
	{0}
};
static const Fl_Menu_Item SolidMenu[] = {
	{"Not solid", 0, solidCB, (void*)0},
	{"Top solid", 0, solidCB, (void*)1},
	{"Left/Right/Bottom solid", 0, solidCB, (void*)2},
	{"All solid", 0, solidCB, (void*)3},
	{0}
};
static const Fl_Menu_Item gameSysMenuSel[] = {
	{"Sega Genesis/Mega Drive", 0, set_game_system, (void*)segaGenesis},
	{"Nintendo Entertainment System/Famicon", 0, set_game_system, (void*)NES},
	{"Master System", 0, set_game_system, (void*)masterSystem},
	{"Game Gear", 0, set_game_system, (void*)gameGear},
	{"TMS9918", 0, set_game_system, (void*)TMS9918},
	{0}
};
extern Fl_Menu_Item subSysGenesis[];
extern const char * MapWidthTxt;
extern const char * MapHeightTxt;
static const char * TooltipZoom = "By changing this slider you are changing the magnification of the tile for example if this slider was set to 10 that would mean that the tile is magnified by a factor of 10";
extern const char*spriteDefName;
extern const char*defMDesc;
extern const char*spritesName;
static void(*const mainCBtab[])(Fl_Widget*, void*) = {
	//Number is plus one because it is that way in the Lua configuration file
	//Subtract one to get the index of said function.
	load_tiles,//1
	load_truecolor_tiles,//2
	save_tiles,//3
	save_tiles_truecolor,//4
	load_image_to_tilemapCB,//5
	loadPalette,//6
	save_palette,//7
	load_tile_map,//8
	save_tilemap_as_image,//9
	save_tilemap_as_colspace,//10
	save_map,//11
	loadProjectCB,//12
	saveProjectCB,//13
	loadAllProjectsCB,//14
	saveAllProjectsCB,//15
	ImportS1CBChunks,//16
	saveChunkS1CB,//17
	SpriteimportCB,//18
	spriteSheetimportCB,//19
	importSonicMappingCB,//20
	importSonicDPLCCB,//21
	exportSonicMappingCB,//22
	exportSonicDPLCCB,//23
	runLuaCB,//24
	generate_optimal_palette,//25
	clearPalette,//26
	pickNearAlg,//27
	rgb_pal_to_entry,//28
	entryToRgb,//29
	sortRowbyCB,//30
	new_tile,//31
	fill_tile,//32
	blank_tile,//33
	remove_duplicate_truecolor,//34
	remove_duplicate_tiles,//35
	update_all_tiles,//36
	delete_tile_at_location,//37
	tilesnewfilppedCB,//38
	tilemap_remove_callback,//39
	trueColTileToggle,//40
	tileDPicker,//41
	shadow_highligh_findout,//42
	dither_tilemap_as_imageCB,//43
	fill_tile_map_with_tile,//44
	FixOutOfRangeCB,//45
	pickExtAttrsCB,//46
	ditherSpriteAsImageCB,//47
	ditherSpriteAsImageAllCB,//48
	optimizeSpritesCB,//49
	palRowstCB,//50
	undoCB,//51
	redoCB,//52
	historyWindow,//53
	clearUndoCB,//54
	showAbout,//55
};
static void callCFuncLua(Fl_Widget*w, void*toPair) {
	std::pair<unsigned, int64_t>*p = (std::pair<unsigned, int64_t>*)toPair;
	(*mainCBtab[p->first])(w, (void*)p->second);
}
static void callLuaCB(Fl_Widget*w, void*toPair) {
	std::pair<std::string, int64_t>*p = (std::pair<std::string, int64_t>*)toPair;
	lua_getglobal(Lconf, p->first.c_str());
	lua_pushinteger(Lconf, p->second);
	runLuaFunc(Lconf, 1, 0);
}
void editor::_editor() {
	//create the window
	keepUserdat = new std::vector<std::pair<unsigned, int64_t>>;
	luaCallback = new std::vector<std::pair<std::string, int64_t>>;
	startLuaConf("config.lua", false);
	menu = new Fl_Menu_Bar(0, 0, 800, 24); //Create menubar, items..
	lua_getglobal(Lconf, "generateMenu");
	runLuaFunc(Lconf, 0, 1);
	{
		unsigned menucnt;
		std::vector<Fl_Menu_Item> tmp;

		if (!(menucnt = lua_rawlen(Lconf, -1))) {
			fl_alert("generateMenu must return a table");
			exit(1);
		}

		for (unsigned i = 1; i <= menucnt; ++i) {
			lua_rawgeti(Lconf, -1, i);
			unsigned len = lua_rawlen(Lconf, -1);

			if (len) {
				Fl_Menu_Item mi{0};
				lua_rawgeti(Lconf, -1, 1);
				char*txt = (char*)luaL_optstring(Lconf, -1, 0);

				if (txt)
					txt = strdup(txt);

				mi.text = txt;
				lua_pop(Lconf, 1);

				if (len >= 2) {
					lua_rawgeti(Lconf, -1, 2);
					mi.shortcut_ = luaL_optinteger(Lconf, -1, 0);
					lua_pop(Lconf, 1);
				} else
					mi.shortcut_ = 0;

				int64_t userdat;

				if (len >= 4) {
					lua_rawgeti(Lconf, -1, 4);
					userdat = luaL_optinteger(Lconf, -1, 0);
					lua_pop(Lconf, 1);
				} else
					userdat = 0;

				if (len >= 3) {
					lua_rawgeti(Lconf, -1, 3);

					if (lua_type(Lconf, -1) == LUA_TSTRING) {
						luaCallback->emplace_back(std::make_pair(lua_tostring(Lconf, -1), userdat));
						mi.callback_ = callLuaCB;
						mi.user_data_ = &(*luaCallback)[luaCallback->size() - 1];
					} else if (lua_type(Lconf, -1) == LUA_TNUMBER) {
						int64_t lcall = lua_tointeger(Lconf, -1);

						if (lcall > 0) {
							keepUserdat->emplace_back(std::make_pair(lcall - 1, userdat));
							mi.callback_ = callCFuncLua;
							mi.user_data_ = &(*keepUserdat)[keepUserdat->size() - 1];
						} else
							mi.callback_ = 0;
					} else {
						fl_alert("Error invalid type for callback %s", lua_typename(Lconf, lua_type(Lconf, -1)));
						exit(1);
					}

					lua_pop(Lconf, 1);
				} else {
					mi.callback_ = 0;
					mi.user_data_ = 0;
				}

				if (len >= 5) {
					lua_rawgeti(Lconf, -1, 5);
					mi.flags = luaL_optinteger(Lconf, -1, 0);
					lua_pop(Lconf, 1);
				} else
					mi.flags = 0;

				tmp.emplace_back(mi);
			} else
				tmp.emplace_back(Fl_Menu_Item{0});
			lua_pop(Lconf, 1);
		}

		unsigned cidx = 0, lidx = 0;

		for (unsigned i = 0; i < menucnt; ++i) { //While resizing the vector the addresses may have changed.
			lua_rawgeti(Lconf, -1, i + 1);
			unsigned len = lua_rawlen(Lconf, -1);

			if (len >= 3) {
				lua_rawgeti(Lconf, -1, 3);

				if (lua_type(Lconf, -1) == LUA_TSTRING)
					tmp[i].user_data_ = &(*luaCallback)[lidx++];

				else if (lua_type(Lconf, -1) == LUA_TNUMBER) {
					int64_t lcall = lua_tointeger(Lconf, -1);

					if (lcall > 0)
						tmp[i].user_data_ = &(*keepUserdat)[cidx++];
				}

				lua_pop(Lconf, 1);
			}

			lua_pop(Lconf, 1);
		}

		lua_pop(Lconf, 1);
		menu->copy(tmp.data());
	}
	tile_placer_tile_offset_y = default_tile_placer_tile_offset_y;
	true_color_box_x = default_true_color_box_x;
	true_color_box_y = default_true_color_box_y;
	tile_edit_truecolor_off_x = default_tile_edit_truecolor_off_x;
	tile_edit_truecolor_off_y = default_tile_edit_truecolor_off_y;
	std::fill(tabsHidden, &tabsHidden[shareAmtPj], false);
	{
		the_tabs = new Fl_Tabs(0, 24, 800, 576);
		the_tabs->callback(set_mode_tabs);
		int rx, ry, rw, rh;
		the_tabs->client_area(rx, ry, rw, rh);
		{
			tabsMain.emplace_back(new Fl_Group(rx, ry, rw, rh, "Palette editor"));
			//stuff related to this group should go here
			palBar.addTab(0, true);
			pal_size = new Fl_Hor_Value_Slider(128, 384, 320, 24, "Palette box size");
			pal_size->minimum(1);
			pal_size->maximum(42);
			pal_size->step(1);
			pal_size->value(32);
			pal_size->align(FL_ALIGN_LEFT);
			pal_size->callback(redrawOnlyCB);
			ditherPower = new Fl_Hor_Value_Slider(128, 416, 320, 24, "Dither Power");
			ditherPower->tooltip("A lower value results in more dithering artifacts a higher value results in less artifacts");
			ditherPower->minimum(1);
			ditherPower->maximum(256);
			ditherPower->step(1);
			ditherPower->value(16);
			ditherPower->align(FL_ALIGN_LEFT);
			ditherPower->callback(setSubditherSetting);
			gameSysSel = new Fl_Choice(96, 312, 192, 24);
			gameSysSel->copy(gameSysMenuSel);
			ditherAlgSel = new Fl_Choice(64, 464, 144, 24);
			ditherAlgSel->copy(ditherChoices);
			subSysC = new Fl_Choice(208, 464, 128, 24);
			subSysC->copy(subSysGenesis);
			subSysC->value(0);
			{	Fl_Group *o = new Fl_Group(306, 188, 88, 96);
				{
					palType[0] = new Fl_Round_Button(306, 188, 64, 32, "Free");
					palType[0]->type(FL_RADIO_BUTTON);
					palType[0]->set();
					palType[0]->callback((Fl_Callback*) setPalType, (void *)0);
					palType[0]->tooltip(freeDes);
					palType[1] = new Fl_Round_Button(306, 220, 72, 32, "Locked");
					palType[1]->type(FL_RADIO_BUTTON);
					palType[1]->callback((Fl_Callback*) setPalType, (void *)1);
					palType[1]->tooltip(lockedDes);
					palType[2] = new Fl_Round_Button(306, 252, 88, 32, "Reserved");
					palType[2]->type(FL_RADIO_BUTTON);
					palType[2]->callback((Fl_Callback*) setPalType, (void *)2);
					palType[2]->tooltip(reservedDes);
					o->end();
				} // End of buttons
			}//end of group

			lua_getglobal(Lconf, "tabConfig");
			lua_pushinteger(Lconf, pal_edit);
			runLuaFunc(Lconf, 1, 0);

			tabsMain[pal_edit]->end();
		} // Fl_Group* o
		{	tabsMain.emplace_back(new Fl_Group(rx, ry, rw, rh, "Tile editor"));
			//stuff related to this group should go here
			{	Fl_Group* o = new Fl_Group(0, 0, 800, 567);
				palRTE[0] = new Fl_Round_Button(384, default_palette_bar_offset_y + 40, 56, 32, "Row 0");
				palRTE[0]->type(FL_RADIO_BUTTON);
				palRTE[0]->set();
				palRTE[0]->callback((Fl_Callback*) set_tile_row, (void *)0);
				palRTE[1] = new Fl_Round_Button(448, default_palette_bar_offset_y + 40, 56, 32, "Row 1");
				palRTE[1]->type(FL_RADIO_BUTTON);
				palRTE[1]->callback((Fl_Callback*) set_tile_row, (void *)1);
				palRTE[2] = new Fl_Round_Button(512, default_palette_bar_offset_y + 40, 56, 32, "Row 2");
				palRTE[2]->type(FL_RADIO_BUTTON);
				palRTE[2]->callback((Fl_Callback*) set_tile_row, (void *)2);
				palRTE[3] = new Fl_Round_Button(576, default_palette_bar_offset_y + 40, 56, 32, "Row 3");
				palRTE[3]->type(FL_RADIO_BUTTON);
				palRTE[3]->callback((Fl_Callback*) set_tile_row, (void *)3);
				o->end();
			} // Fl_Group* o
			{	Fl_Button *o = new Fl_Button(538, default_palette_bar_offset_y, 104, 32, "Append tile");//these button should be inline with the palette bar
				o->tooltip("This will append a blank tile to the tile buffer in the ram.");
				o->callback(new_tile);
			}
			{	Fl_Button *o = new Fl_Button(656, default_palette_bar_offset_y, 140, 32, "Delete selected tile");
				o->tooltip("This button will delete the currently selected tile");
				o->callback(delete_tile_at_location);
				o->labelsize(12);
			}
			{	Fl_Button *o = new Fl_Button(656, default_palette_bar_offset_y + 34, 140, 32, "Insert after tile");
				o->callback(insertTileCB);
			}
			palBar.addTab(1);
			rgb_red = new Fl_Hor_Value_Slider(64, default_palette_bar_offset_y + 136, 128, 24, "RGB red");
			rgb_red->minimum(0);
			rgb_red->maximum(255);
			rgb_red->step(1);
			rgb_red->value(0);
			rgb_red->align(FL_ALIGN_LEFT);
			rgb_red->callback(update_truecolor, (void *)0);
			rgb_green = new Fl_Hor_Value_Slider(240, default_palette_bar_offset_y + 136, 128, 24, "Green");
			rgb_green->minimum(0);
			rgb_green->maximum(255);
			rgb_green->step(1);
			rgb_green->value(0);
			rgb_green->align(FL_ALIGN_LEFT);
			rgb_green->callback(update_truecolor, (void *)1);
			rgb_blue = new Fl_Hor_Value_Slider(402, default_palette_bar_offset_y + 136, 128, 24, "Blue");
			rgb_blue->minimum(0);
			rgb_blue->maximum(255);
			rgb_blue->step(1);
			rgb_blue->value(0);
			rgb_blue->align(FL_ALIGN_LEFT);
			rgb_blue->callback(update_truecolor, (void *)2);
			rgb_alpha = new Fl_Hor_Value_Slider(576, default_palette_bar_offset_y + 136, 128, 24, "Alpha");
			rgb_alpha->minimum(0);
			rgb_alpha->maximum(255);
			rgb_alpha->step(1);
			rgb_alpha->value(0);
			rgb_alpha->align(FL_ALIGN_LEFT);
			rgb_alpha->callback(update_truecolor, (void *)3);
			{	Fl_Group *o = new Fl_Group(304, 96, 88, 96);
				{
					palType[3] = new Fl_Round_Button(304, 96, 64, 32, "Free");
					palType[3]->type(FL_RADIO_BUTTON);
					palType[3]->set();
					palType[3]->callback((Fl_Callback*) setPalType, (void *)0);
					palType[3]->tooltip(freeDes);
					palType[4] = new Fl_Round_Button(304, 128, 72, 32, "Locked");
					palType[4]->type(FL_RADIO_BUTTON);
					palType[4]->callback((Fl_Callback*) setPalType, (void *)1);
					palType[4]->tooltip(lockedDes);
					palType[5] = new Fl_Round_Button(304, 160, 88, 32, "Reserved");
					palType[5]->type(FL_RADIO_BUTTON);
					palType[5]->callback((Fl_Callback*) setPalType, (void *)2);
					palType[5]->tooltip(reservedDes);
					o->end();
				} // End of buttons
			}//end of group
			{	Fl_Check_Button* o = new Fl_Check_Button(694, default_palette_bar_offset_y + 68, 100, 32, "Show grid?");
				o->callback(set_grid);
				o->tooltip("This button Toggles weather or not you which to see a grid while editing your tiles. A grid can help you see the spacing between each pixel.");
			}
			tile_edit_offset_x = default_tile_edit_offset_x;
			tile_edit_offset_y = default_tile_edit_offset_y;
			tile_size = new Fl_Hor_Value_Slider(448, default_palette_bar_offset_y + 72, 242, 24, "Tile zoom");
			tile_size->tooltip(TooltipZoom);
			tile_size->minimum(1);
			tile_size->maximum(64);
			tile_size->step(1);
			tile_size->value(46);
			tile_size->align(FL_ALIGN_LEFT);
			tile_size->callback(update_offset_tile_edit);
			//now for the tile select slider
			tile_select = new Fl_Hor_Value_Slider(480, default_palette_bar_offset_y + 104, 312, 24, "Tile select");
			tile_select->tooltip("This slider selects which tile that you are editing the first tile is zero");
			tile_select->minimum(0);
			tile_select->maximum(0);
			tile_select->step(1);
			tile_select->align(FL_ALIGN_LEFT);
			tile_select->callback(set_tile_current);
			lua_getglobal(Lconf, "tabConfig");
			lua_pushinteger(Lconf, tile_edit);
			runLuaFunc(Lconf, 1, 0);
			tabsMain[tile_edit]->end();
		}
		{	tabsMain.emplace_back(new Fl_Group(rx, ry, rw, rh, "Plane mapping/block editor"));
			{	Fl_Group* o = new Fl_Group(tile_place_buttons_x_off, 192, 60, 128);
				palRTE[4] = new Fl_Round_Button(tile_place_buttons_x_off, 192, 60, 28, "Row 0");
				palRTE[4]->type(FL_RADIO_BUTTON);
				palRTE[4]->set();
				palRTE[4]->callback((Fl_Callback*) set_tile_row, (void *)0);
				palRTE[5] = new Fl_Round_Button(tile_place_buttons_x_off, 220, 60, 28, "Row 1");
				palRTE[5]->type(FL_RADIO_BUTTON);
				palRTE[5]->callback((Fl_Callback*) set_tile_row, (void *)1);
				palRTE[6] = new Fl_Round_Button(tile_place_buttons_x_off, 248, 60, 28, "Row 2");
				palRTE[6]->type(FL_RADIO_BUTTON);
				palRTE[6]->callback((Fl_Callback*) set_tile_row, (void *)2);
				palRTE[7] = new Fl_Round_Button(tile_place_buttons_x_off, 276, 60, 28, "Row 3");
				palRTE[7]->type(FL_RADIO_BUTTON);
				palRTE[7]->callback((Fl_Callback*) set_tile_row, (void *)3);
				o->end();
			} // Fl_Group* o


			planeSelect = new Fl_Choice(408, default_palette_bar_offset_y + 56, 112, 24, "Plane selection");
			planeSelect->align(FL_ALIGN_TOP);

			{
				Fl_Button *o = new Fl_Button(408, default_palette_bar_offset_y + 80, 112, 24, "Add plane");
				o->callback(addPlaneTilemap);
			}

			{
				Fl_Button *o = new Fl_Button(408, default_palette_bar_offset_y + 104, 112, 24, "Remove plane");
				o->callback(removeTilemapsPlane);
			}

			curPlaneName = new Fl_Input(tile_place_buttons_x_off + 616, 56, 168, 24, "Plane name");
			curPlaneName->value("0");
			curPlaneName->callback(updateNameTilemaps);

			map_w = new Fl_Int_Input(608, default_palette_bar_offset_y + 72, 184, 24, MapWidthTxt);
			map_w->when(FL_WHEN_ENTER_KEY);
			map_w->value("2");
			map_w->align(FL_ALIGN_LEFT);
			map_w->callback(callback_resize_map);
			map_h = new Fl_Int_Input(608, default_palette_bar_offset_y + 104, 184, 24, MapHeightTxt);
			map_h->when(FL_WHEN_ENTER_KEY);
			map_h->value("2");
			map_h->align(FL_ALIGN_LEFT);
			map_h->callback(callback_resize_map);
			map_amt = new Fl_Int_Input(480, default_palette_bar_offset_y + 136, 312, 24, "Blocks");
			map_amt->value("1");
			map_amt->align(FL_ALIGN_LEFT);
			map_amt->callback(blocksAmtCB);
			map_amt->hide();
			map_x_scroll = new Fl_Scrollbar(default_map_off_x - 32, default_map_off_y - 32, 800 - 8 - default_map_off_x, 24);
			map_x_scroll->value(0, 0, 0, 0);
			map_x_scroll->type(FL_HORIZONTAL);
			map_x_scroll->tooltip("Use this scroll bar to move around the tile map if you are zoomed in and there is not enough room to display the entire tilemap at once. This scroll bar will move the map left and right.");
			map_x_scroll->callback(update_map_scroll_x);
			map_x_scroll->hide();
			map_x_scroll->linesize(1);
			map_y_scroll = new Fl_Scrollbar(default_map_off_x - 32, default_map_off_y, 24, 600 - 8 - default_map_off_y);
			map_y_scroll->value(0, 0, 0, 0);
			//map_x_scroll->type(FL_HORIZONTAL);
			map_y_scroll->tooltip("Use this scroll bar to move around the tile map if you are zoomed in and there is not enough room to display the entire tilemap at once. This scroll bar will move the map up and down.");
			map_y_scroll->callback(update_map_scroll_y);
			map_y_scroll->hide();
			map_y_scroll->linesize(1);
			//now for the tile select slider
			tile_select_2 = new Fl_Hor_Value_Slider(528, default_palette_bar_offset_y + 40, 264, 24, "Tile select");
			tile_select_2->tooltip("This slider allows you to choice which tile you would like to place on the map remember you can both horizontally and vertically flip the tile once placed on the map and select which row the tile uses");
			tile_select_2->minimum(0);
			tile_select_2->maximum(0);
			tile_select_2->step(1);
			tile_select_2->align(FL_ALIGN_TOP_LEFT);
			tile_select_2->callback(set_tile_currentTP);
			totalTiles = new Fl_Box(600, default_palette_bar_offset_y, 128, 64);
			totalTiles->labelsize(12);
			palBar.addTab(2);
			//buttons for tile settings
			{	Fl_Group *o = new Fl_Group(304, 96, 88, 96);
				{
					palType[6] = new Fl_Round_Button(304, 90, 64, 32, "Free");
					palType[6]->type(FL_RADIO_BUTTON);
					palType[6]->set();
					palType[6]->callback((Fl_Callback*) setPalType, (void *)0);
					palType[6]->tooltip(freeDes);
					palType[7] = new Fl_Round_Button(304, 122, 72, 32, "Locked");
					palType[7]->type(FL_RADIO_BUTTON);
					palType[7]->callback((Fl_Callback*) setPalType, (void *)1);
					palType[7]->tooltip(lockedDes);
					palType[8] = new Fl_Round_Button(304, 154, 88, 32, "Reserved");
					palType[8]->type(FL_RADIO_BUTTON);
					palType[8]->callback((Fl_Callback*) setPalType, (void *)2);
					palType[8]->tooltip(reservedDes);
					o->end();
				} // End of buttons
			}//end of group

			hflipCB[0] = new Fl_Check_Button(tile_place_buttons_x_off, 304, 64, 32, "Hflip");
			hflipCB[0]->callback(set_hflipCB, (void*)0);
			hflipCB[0]->tooltip("This sets whether or not the tile is flipped horizontally");
			vflipCB[0] = new Fl_Check_Button(tile_place_buttons_x_off, 336, 64, 32, "Vflip");
			vflipCB[0]->callback(set_vflipCB, (void*)0);
			vflipCB[0]->tooltip("This sets whether or not the tile is flipped vertically");
			prioCB[0] = new Fl_Check_Button(tile_place_buttons_x_off, 368, 72, 32, "Priority");
			prioCB[0]->callback(set_prioCB, (void*)0);
			prioCB[0]->tooltip("If checked tile is high priority");
			{	Fl_Check_Button* o = new Fl_Check_Button(tile_place_buttons_x_off, 400, 96, 32, "Show grid?");
				o->callback(set_grid_placer);
				o->tooltip("This button toggles whether or not a grid is visible over the tile map this will allow you to easily see were each tile is");
			}
			BlocksCBtn = new Fl_Check_Button(tile_place_buttons_x_off, 432, 96, 32, "Blocks?");
			BlocksCBtn->callback(toggleBlocksCB);
			BlocksCBtn->tooltip("Toggles if tile map is treated as blocks");
			{	Fl_Check_Button* o = new Fl_Check_Button(tile_place_buttons_x_off, 464, 192, 32, "Show only selected row");
				o->callback(toggleRowSolo);
				o->tooltip("When checked tiles that do not use the selected row will not be drawn");
			}
			place_tile_size = new Fl_Hor_Value_Slider(tile_place_buttons_x_off, 512, 168, 24, "Tile zoom factor:");
			place_tile_size->minimum(1);
			place_tile_size->maximum(16);
			place_tile_size->step(1);
			place_tile_size->value(12);
			place_tile_size->align(FL_ALIGN_TOP);
			place_tile_size->callback(update_map_size);
			place_tile_size->tooltip(TooltipZoom);

			tmapOffset = new Fl_Int_Input(tile_place_buttons_x_off, 552, 168, 24, "Tile offset");
			tmapOffset->when(FL_WHEN_ENTER_KEY);
			tmapOffset->value("0");
			tmapOffset->align(FL_ALIGN_TOP);
			tmapOffset->callback(setTmapOffsetCB);

			cordDisp[0] = new Fl_Box(tile_place_buttons_x_off, 556, 128, 64);
			cordDisp[0]->labelsize(12);

			lua_getglobal(Lconf, "tabConfig");
			lua_pushinteger(Lconf, tile_place);
			runLuaFunc(Lconf, 1, 0);
			tabsMain[tile_place]->end();
		}
		{	tabsMain.emplace_back(new Fl_Group(rx, ry, rw, rh, "Chunk editor"));
			useBlocksChunkCBtn = new Fl_Check_Button(8, 48, 152, 24, "Use blocks");
			useBlocksChunkCBtn->callback(useBlocksCB);

			chunkX = new Fl_Scrollbar(DefaultChunkX - 32, DefaultChunkY - 32, 800 - DefaultChunkX + 24, 24);
			chunkX->value(0, 0, 0, 0);
			chunkX->type(FL_HORIZONTAL);
			chunkX->callback(scrollChunkX);
			chunkX->hide();

			chunkY = new Fl_Scrollbar(DefaultChunkX - 32, DefaultChunkY, 24, 600 - 8 - DefaultChunkY);
			chunkY->value(0, 0, 0, 0);
			chunkY->callback(scrollChunkY);
			chunkY->hide();

			chunk_select = new Fl_Hor_Value_Slider(tile_place_buttons_x_off, 88, 160, 24, "Chunk select");
			chunk_select->minimum(0);
			chunk_select->maximum(0);
			chunk_select->step(1);
			chunk_select->value(0);
			chunk_select->align(FL_ALIGN_TOP);
			chunk_select->callback(currentChunkCB);

			tile_select_3 = new Fl_Hor_Value_Slider(tile_place_buttons_x_off, 136, 160, 24, "Tile select");
			tile_select_3->minimum(0);
			tile_select_3->maximum(0);
			tile_select_3->step(1);
			tile_select_3->align(FL_ALIGN_TOP);
			tile_select_3->callback(selBlockCB);

			hflipCB[1] = new Fl_Check_Button(tile_place_buttons_x_off, 160, 64, 32, "Hflip");
			hflipCB[1]->callback(set_hflipCB, (void*)1);
			vflipCB[1] = new Fl_Check_Button(tile_place_buttons_x_off, 192, 64, 32, "Vflip");
			vflipCB[1]->callback(set_vflipCB, (void*)1);
			prioCB[1] = new Fl_Check_Button(tile_place_buttons_x_off, 224, 72, 32, "Priority");
			prioCB[1]->callback(set_prioCB, (void*)1);

			solidChunkMenu = new Fl_Choice(tile_place_buttons_x_off, 256, 128, 24);
			solidChunkMenu->copy(SolidMenu);

			chunksize[0] = new Fl_Int_Input(tile_place_buttons_x_off, 296, 128, 24, "Width (in tiles)");
			chunksize[0]->when(FL_WHEN_ENTER_KEY);
			chunksize[0]->align(FL_ALIGN_TOP);
			chunksize[0]->callback(resizeChunkCB);
			chunksize[0]->value("16");

			chunksize[1] = new Fl_Int_Input(tile_place_buttons_x_off, 336, 128, 24, "Height (in tiles)");
			chunksize[1]->when(FL_WHEN_ENTER_KEY);
			chunksize[1]->align(FL_ALIGN_TOP);
			chunksize[1]->callback(resizeChunkCB);
			chunksize[1]->value("16");

			planeSelectChunk = new Fl_Hor_Value_Slider(tile_place_buttons_x_off, 480, 160, 24, "Plane select");
			planeSelectChunk->minimum(0);
			planeSelectChunk->maximum(0);
			planeSelectChunk->step(1);
			planeSelectChunk->align(FL_ALIGN_TOP);
			planeSelectChunk->callback(setCurPlaneChunkCB);

			chunk_tile_size = new Fl_Hor_Value_Slider(tile_place_buttons_x_off, 520, 160, 24, "Tile zoom factor:");
			chunk_tile_size->minimum(1);
			chunk_tile_size->maximum(16);
			chunk_tile_size->step(1);
			chunk_tile_size->value(2);
			chunk_tile_size->align(FL_ALIGN_TOP);
			chunk_tile_size->callback(scrollChunkCB);
			chunk_tile_size->tooltip(TooltipZoom);

			cordDisp[1] = new Fl_Box(tile_place_buttons_x_off, 556, 128, 64);
			cordDisp[1]->labelsize(12);

			{	Fl_Button *o = new Fl_Button(tile_place_buttons_x_off, 364, 112, 32, "Append chunk");
				o->callback(appendChunkCB);
			}

			{	Fl_Button *o = new Fl_Button(tile_place_buttons_x_off, 396, 128, 32, "Insert after chunk");
				o->callback(insertChunkCB);
			}
			{	Fl_Button *o = new Fl_Button(tile_place_buttons_x_off, 428, 160, 32, "Delete selected chunk");
				o->callback(delChunkAtCB);
			}
			updateChunkSize();

			lua_getglobal(Lconf, "tabConfig");
			lua_pushinteger(Lconf, chunkEditor);
			runLuaFunc(Lconf, 1, 0);
			tabsMain[chunkEditor]->end();
		}
		{	tabsMain.emplace_back(new Fl_Group(rx, ry, rw, rh, "Sprites"));
			palBar.addTab(3, false, true, true);
			{	Fl_Group *o = new Fl_Group(tile_place_buttons_x_off + 616, 44, 88, 96);
				{
					palType[9] = new Fl_Round_Button(tile_place_buttons_x_off + 616, 44, 64, 24, "Free");
					palType[9]->type(FL_RADIO_BUTTON);
					palType[9]->set();
					palType[9]->callback((Fl_Callback*) setPalType, (void *)0);
					palType[9]->tooltip(freeDes);
					palType[10] = new Fl_Round_Button(tile_place_buttons_x_off + 616, 68, 72, 24, "Locked");
					palType[10]->type(FL_RADIO_BUTTON);
					palType[10]->callback((Fl_Callback*) setPalType, (void *)1);
					palType[10]->tooltip(lockedDes);
					palType[11] = new Fl_Round_Button(tile_place_buttons_x_off + 616, 92, 88, 24, "Reserved");
					palType[11]->type(FL_RADIO_BUTTON);
					palType[11]->callback((Fl_Callback*) setPalType, (void *)2);
					palType[11]->tooltip(reservedDes);
					o->end();
				}
			}

			spritegrouptxt = new Fl_Input(tile_place_buttons_x_off + 616, 128, 168, 24, "Group name");
			spritegrouptxt->align(FL_ALIGN_TOP);
			spritegrouptxt->value(spriteDefName);
			spritegrouptxt->callback(assignSpritegroupnameCB);

			spritemetatxt = new Fl_Input(tile_place_buttons_x_off + 616, 168, 168, 24, "Meta name");
			spritemetatxt->value(spritesName);
			spritemetatxt->callback(assignSpritemetaNameCB);
			spritemetatxt->align(FL_ALIGN_TOP);

			spriteglobaltxt = new Fl_Input(tile_place_buttons_x_off + 616, 208, 168, 24, "All meta name");
			spriteglobaltxt->value(defMDesc);
			spriteglobaltxt->callback(assignSpriteAllMetanameCB);
			spriteglobaltxt->align(FL_ALIGN_TOP);


			metaspritesel = new Fl_Hor_Value_Slider(tile_place_buttons_x_off + 616, 244, 168, 24, "Meta group select:");
			metaspritesel->step(1);
			metaspritesel->maximum(0);
			metaspritesel->align(FL_ALIGN_TOP);
			metaspritesel->callback(selspriteMeta);
			metaspritesel->labelsize(12);


			{	Fl_Button *o = new Fl_Button(tile_place_buttons_x_off + 616, 274, 96, 28, "Append meta");
				o->callback(appendSpriteCB, (void*)(intptr_t)2);
			}

			spriteselgroup = new Fl_Hor_Value_Slider(tile_place_buttons_x_off, 184, 168, 22, "Sprite group select:");
			spriteselgroup->step(1);
			spriteselgroup->maximum(0);
			spriteselgroup->align(FL_ALIGN_TOP);
			spriteselgroup->callback(selspriteGroup);
			spriteselgroup->labelsize(12);

			spritesel = new Fl_Hor_Value_Slider(tile_place_buttons_x_off, 220, 168, 22, "Sprite select:");
			spritesel->step(1);
			spritesel->maximum(0);
			spritesel->align(FL_ALIGN_TOP);
			spritesel->callback(selSpriteCB);
			spritesel->labelsize(12);

			spritest = new Fl_Hor_Value_Slider(tile_place_buttons_x_off, 256, 168, 22, "Start tile:");
			spritest->step(1);
			spritest->maximum(0);
			spritest->align(FL_ALIGN_TOP);
			spritest->callback(setvalueSpriteCB, 0);
			spritest->labelsize(12);

			spriteslat = new Fl_Hor_Value_Slider(tile_place_buttons_x_off, 292, 168, 22, "Mapping tile");
			spriteslat->step(1);
			spriteslat->maximum(0);
			spriteslat->align(FL_ALIGN_TOP);
			spriteslat->callback(setvalueSpriteCB, (void*)4);
			spriteslat->labelsize(12);

			spritesize[0] = new Fl_Hor_Value_Slider(tile_place_buttons_x_off + 40, 316, 128, 22, "Width");
			spritesize[0]->step(1);
			spritesize[0]->value(1);
			spritesize[0]->minimum(1);
			spritesize[0]->maximum(4);
			spritesize[0]->align(FL_ALIGN_LEFT);
			spritesize[0]->callback(setvalueSpriteCB, (void*)1);
			spritesize[0]->labelsize(12);

			spritesize[1] = new Fl_Hor_Value_Slider(tile_place_buttons_x_off + 40, 340, 128, 22, "Height");
			spritesize[1]->step(1);
			spritesize[1]->value(1);
			spritesize[1]->minimum(1);
			spritesize[1]->maximum(4);
			spritesize[1]->align(FL_ALIGN_LEFT);
			spritesize[1]->callback(setvalueSpriteCB, (void*)2);
			spritesize[1]->labelsize(12);

			spritepalrow = new Fl_Hor_Value_Slider(tile_place_buttons_x_off, 376, 168, 22, "Palette row:");
			spritepalrow->step(1);
			spritepalrow->maximum(3);
			spritepalrow->align(FL_ALIGN_TOP);
			spritepalrow->callback(setvalueSpriteCB, (void*)3);
			spritepalrow->labelsize(12);

			spritezoom = new Fl_Hor_Value_Slider(tile_place_buttons_x_off + 38, 400, 130, 22, "Zoom");
			spritezoom->step(1);
			spritezoom->minimum(1);
			spritezoom->value(16);
			spritezoom->maximum(16);
			spritezoom->align(FL_ALIGN_LEFT);
			spritezoom->callback(redrawOnlyCB);
			spritezoom->labelsize(12);

			spritesoff[0] = new Fl_Int_Input(tile_place_buttons_x_off + 62, 424, 106, 24, "Offset X:");
			spritesoff[0]->when(FL_WHEN_ENTER_KEY);
			spritesoff[0]->value("0");
			spritesoff[0]->align(FL_ALIGN_LEFT);
			spritesoff[0]->callback(setoffspriteCB, 0);

			spritesoff[1] = new Fl_Int_Input(tile_place_buttons_x_off + 62, 448, 106, 24, "Offset Y:");
			spritesoff[1]->when(FL_WHEN_ENTER_KEY);
			spritesoff[1]->value("0");
			spritesoff[1]->align(FL_ALIGN_LEFT);
			spritesoff[1]->callback(setoffspriteCB, (void*)1);

			spritehflip = new Fl_Check_Button(tile_place_buttons_x_off, 470, 48, 20, "Hflip");
			spritehflip->callback(spriteHflipCB);
			spritevflip = new Fl_Check_Button(tile_place_buttons_x_off + 52, 470, 48, 20, "Vflip");
			spritevflip->callback(spriteVflipCB);
			spriteprio = new Fl_Check_Button(tile_place_buttons_x_off + 104, 470, 56, 20, "Priority");
			spriteprio->callback(spritePrioCB);

			{	Fl_Button *o = new Fl_Button(tile_place_buttons_x_off, 492, 64, 28, "Append");
				o->callback(appendSpriteCB, 0);
				o->labelsize(12);
			}
			{	Fl_Button *o = new Fl_Button(tile_place_buttons_x_off + 72, 492, 96, 28, "Append group");
				o->callback(appendSpriteCB, (void*)(intptr_t)1);
				o->labelsize(12);
			}
			{	Fl_Button *o = new Fl_Button(tile_place_buttons_x_off, 522, 64, 28, "Delete");
				o->callback(delSpriteCB, 0);
				o->labelsize(12);
			}
			{	Fl_Button *o = new Fl_Button(tile_place_buttons_x_off + 72, 522, 96, 28, "Delete group");
				o->callback(delSpriteCB, (void*)1);
				o->labelsize(12);
			}
			spritealign[0] = new Fl_Button(tile_place_buttons_x_off, 552, 32, 28, "Left");
			spritealign[0]->labelsize(12);
			spritealign[0]->callback(alignSpriteCB, (void*)0);

			spritealign[1] = new Fl_Button(tile_place_buttons_x_off + 34, 552, 40, 28, "Right");
			spritealign[1]->labelsize(12);
			spritealign[1]->callback(alignSpriteCB, (void*)1);

			spritealign[2] = new Fl_Button(tile_place_buttons_x_off + 78, 552, 28, 28, "Top");
			spritealign[2]->labelsize(12);
			spritealign[2]->callback(alignSpriteCB, (void*)2);

			spritealign[3] = new Fl_Button(tile_place_buttons_x_off + 112, 552, 48, 28, "Bottom");
			spritealign[3]->labelsize(12);
			spritealign[3]->callback(alignSpriteCB, (void*)3);

			{
				Fl_Group *o = new Fl_Group(tile_place_buttons_x_off, 572, 800, 480);
				{
					Fl_Round_Button*m = new Fl_Round_Button(tile_place_buttons_x_off, 572, 96, 32, "Top corner");
					m->type(FL_RADIO_BUTTON);
					m->callback(setDrawSpriteCB, (void *)0);
					m->set();
				} // Fl_Round_Button* o
				{
					Fl_Round_Button*m = new Fl_Round_Button(tile_place_buttons_x_off + 96, 572, 64, 32, "Center");
					m->type(FL_RADIO_BUTTON);
					m->callback(setDrawSpriteCB, (void *)1);
				} // Fl_Round_Button* o
				o->end();
			} // End of buttons

			lua_getglobal(Lconf, "tabConfig");
			lua_pushinteger(Lconf, spriteEditor);
			runLuaFunc(Lconf, 1, 0);
			tabsMain[spriteEditor]->end();
		}
		{	tabsMain.emplace_back(new Fl_Group(rx, ry, rw, rh, "Level editor"));
			lua_getglobal(Lconf, "tabConfig");
			lua_pushinteger(Lconf, levelEditor);
			runLuaFunc(Lconf, 1, 0);
			tabsMain[levelEditor]->end();
		}
		{	tabsMain.emplace_back(new Fl_Group(rx, ry, rw, rh, "Settings/projects"));
			projectSelect = new Fl_Hor_Value_Slider(112, 56, 128, 24, "Current project");
			projectSelect->minimum(0);
			projectSelect->maximum(0);
			projectSelect->step(1);
			projectSelect->value(0);
			projectSelect->align(FL_ALIGN_LEFT);
			projectSelect->callback(switchProjectCB);
			{	Fl_Button *o = new Fl_Button(260, 52, 152, 32, "Append blank project");
				o->callback(appendProjectCB);
			}
			{	Fl_Button *o = new Fl_Button(428, 52, 168, 32, "Delete selected project");
				o->callback(deleteProjectCB);
			}
			//IMPORTANT if adding a new tab remember to update these
			sharePrj[0] = new Fl_Check_Button(8, 112, 112, 16, "Share palette");
			sharePrj[0]->callback(shareProjectCB, (void*)pjHavePal);
			sharePrj[1] = new Fl_Check_Button(120, 112, 96, 16, "Share tiles");
			sharePrj[1]->callback(shareProjectCB, (void*)pjHaveTiles);
			sharePrj[2] = new Fl_Check_Button(216, 112, 120, 16, "Share tile map");
			sharePrj[2]->callback(shareProjectCB, (void*)pjHaveMap);
			sharePrj[3] = new Fl_Check_Button(336, 112, 120, 16, "Share chunks");
			sharePrj[3]->callback(shareProjectCB, (void*)pjHaveChunks);
			sharePrj[4] = new Fl_Check_Button(456, 112, 120, 16, "Share sprites");
			sharePrj[4]->callback(shareProjectCB, (void*)pjHaveSprites);
			sharePrj[5] = new Fl_Check_Button(576, 112, 120, 16, "Share level");
			sharePrj[5]->callback(shareProjectCB, (void*)pjHaveLevel);

			havePrj[0] = new Fl_Check_Button(8, 88, 112, 16, "Have palette");
			havePrj[0]->callback(haveCB, (void*)pjHavePal);
			havePrj[1] = new Fl_Check_Button(120, 88, 96, 16, "Have tiles");
			havePrj[1]->callback(haveCB, (void*)pjHaveTiles);
			havePrj[2] = new Fl_Check_Button(232, 88, 120, 16, "Have tile map");
			havePrj[2]->callback(haveCB, (void*)pjHaveMap);
			havePrj[3] = new Fl_Check_Button(344, 88, 120, 16, "Have chunks");
			havePrj[3]->callback(haveCB, (void*)pjHaveChunks);
			havePrj[4] = new Fl_Check_Button(456, 88, 120, 16, "Have sprites");
			havePrj[4]->callback(haveCB, (void*)pjHaveSprites);
			havePrj[5] = new Fl_Check_Button(568, 88, 120, 16, "Have level");
			havePrj[5]->callback(haveCB, (void*)pjHaveLevel);

			shareWith[0] = new Fl_Hor_Value_Slider(8, 142, 128, 24, "Share palette with:");
			shareWith[0]->callback(switchShareCB, (void*)pjHavePal);
			shareWith[1] = new Fl_Hor_Value_Slider(136, 142, 128, 24, "Share tiles with:");
			shareWith[1]->callback(switchShareCB, (void*)pjHaveTiles);
			shareWith[2] = new Fl_Hor_Value_Slider(264, 142, 128, 24, "Share tile map with:");
			shareWith[2]->callback(switchShareCB, (void*)pjHaveMap);
			shareWith[3] = new Fl_Hor_Value_Slider(400, 142, 128, 24, "Share chunks with:");
			shareWith[3]->callback(switchShareCB, (void*)pjHaveChunks);
			shareWith[4] = new Fl_Hor_Value_Slider(536, 142, 128, 24, "Share sprites with:");
			shareWith[4]->callback(switchShareCB, (void*)pjHaveSprites);
			shareWith[5] = new Fl_Hor_Value_Slider(672, 142, 128, 24, "Share level with:");
			shareWith[5]->callback(switchShareCB, (void*)pjHaveLevel);

			for (unsigned x = 0; x < shareAmtPj; ++x) {
				havePrj[x]->value(1);
				shareWith[x]->minimum(0);
				shareWith[x]->maximum(0);
				shareWith[x]->step(1);
				shareWith[x]->value(0);
				shareWith[x]->align(FL_ALIGN_TOP);
			}


			TxtBufProject = new Fl_Text_Buffer;
			TxtEditProject = new Fl_Text_Editor(8, 184, 640, 370, "Description/Notes");
			TxtEditProject->buffer(TxtBufProject);
			TxtEditProject->textfont(FL_TIMES);
			TxtBufProject->text(currentProject->Name.c_str());
			lua_getglobal(Lconf, "tabConfig");
			lua_pushinteger(Lconf, settingsTab);
			runLuaFunc(Lconf, 1, 0);
			tabsMain[settingsTab]->end();
		}
		{	tabsMain.emplace_back(new Fl_Group(rx, ry, rw, rh, "Lua scripting"));
			luaScriptSel = new Fl_Choice(tile_place_buttons_x_off, default_palette_bar_offset_y + 8, 112, 24);
			luaScriptSel->label("Script selection");
			luaScriptSel->align(FL_ALIGN_TOP);
			luaScriptSel->callback(switchCurLuaScript);
			{	Fl_Button *o = new Fl_Button(tile_place_buttons_x_off + 120, default_palette_bar_offset_y, 112, 32, "Append script");
				o->callback(appendLuaScript);
			}
			{	Fl_Button *o = new Fl_Button(tile_place_buttons_x_off + 240, default_palette_bar_offset_y, 144, 32, "Delete selected script");
				o->callback(deleteLuaScript);
			}
			{	Fl_Button *o = new Fl_Button(tile_place_buttons_x_off + 392, default_palette_bar_offset_y, 48, 32, "Run");
				o->callback(runCurLuaScript);
			}
			luaScriptName = new Fl_Input(tile_place_buttons_x_off + 448 + 8, default_palette_bar_offset_y + 8, 328, 24, "Script name");
			luaScriptName->callback(setNameLuaScript);
			luaScriptName->align(FL_ALIGN_TOP);
			luaBufProject = new Fl_Text_Buffer;
			luaEditProject = new Fl_Text_Editor(tile_place_buttons_x_off, default_palette_bar_offset_y + 48, 784, 456, "Currently selected Lua script");
			luaEditProject->buffer(luaBufProject);
			luaEditProject->textfont(FL_COURIER);
			luaEditProject->hide();
			lua_getglobal(Lconf, "tabConfig");
			lua_pushinteger(Lconf, luaTab);
			runLuaFunc(Lconf, 1, 0);
			tabsMain[luaTab]->end();
		}
	}
}
