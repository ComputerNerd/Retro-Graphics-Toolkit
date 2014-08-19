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
#include "global.h"
#include "callbacks_palette.h"
#include "callback_tiles.h"
#include "tiles_io.h"
#include "callback_tilemap.h"
#include "callback_gui.h"
#include "callback_project.h"
#include "callback_chunk.h"
#include "callbacksprites.h"
#include "undocallback.h"
#include "undo.h"
#include "gamedef.h"
void set_mode_tabs(Fl_Widget* o, void*){
	Fl_Group * val=(Fl_Group*)(Fl_Tabs*)window->the_tabs->value();
	if(val==window->TabsMain[pal_edit]){
		mode_editor=pal_edit;
		palEdit.updateSlider();
	}else if(val==window->TabsMain[tile_edit]){
		currentProject->tileC->current_tile=window->tile_select->value();
		mode_editor=tile_edit;
		tileEdit_pal.updateSlider();
	}else if(val==window->TabsMain[tile_place]){
		currentProject->tileC->current_tile=window->tile_select_2->value();
		mode_editor=tile_place;
		tileMap_pal.updateSlider();
	}else if(val==window->TabsMain[chunkEditor]){
		mode_editor=chunkEditor;
	}else if(val==window->TabsMain[spriteEditor]){
		mode_editor=spriteEditor;
		spritePal.updateSlider();
	}else if(val==window->TabsMain[levelEditor]){
		mode_editor=levelEditor;
	}else if(val==window->TabsMain[settingsTab]){
		mode_editor=settingsTab;
	}
}
static const char * freeDes="This sets the currently selected palette entry to free meaning that this color can be changed";
static const char * lockedDes="This sets the currently selected palette entry to locked meaning that this color cannot be changed but tiles can still use it";
static const char * reservedDes="This sets the currently selected palette entry to reserved meaning that this color cannot be changed or used in tiles note that you may need make sure all tiles get re-dithered to ensure that this rule is enforced";
static const Fl_Menu_Item menuEditor[]={
	{"File",0, 0, 0, FL_SUBMENU},
		{"Tiles",0, 0, 0, FL_SUBMENU},
			{"Open tiles",0,load_tiles,0},
			{"Open truecolor tiles",0,load_truecolor_tiles,0},
			{"Append tiles",0,load_tiles,(void*)1},
			{"Save tiles",0,save_tiles,0},
			{"Save truecolor tiles",0,save_tiles_truecolor,0},
			{"Import image to tiles",0,load_image_to_tilemap,(void*)2},
			{0},
		{"Palettes",0, 0, 0, FL_SUBMENU},
			{"Open palette",0,loadPalette,0},
			{"Save Palette",0, save_palette,0},
			{0},
		{"Tilemaps",0, 0, 0, FL_SUBMENU},
			{"Open tile map or blocks and if NES attributes",0,load_tile_map,0},
			{"Import image to tilemap",0,load_image_to_tilemap,0},
			{"Import image over current tilemap",0,load_image_to_tilemap,(void*)1},
			{"Save tilemap as image",0,save_tilemap_as_image,0},
			{"Save tilemap as with system color space",0,save_tilemap_as_colspace,0},
			{"Save tile map and if nes attributes",0,save_map,0},
			{0},
		{"Projects",0, 0, 0, FL_SUBMENU},
			{"Load project",FL_CTRL+FL_SHIFT+'o',loadProjectCB,0},
			{"Save project",FL_CTRL+FL_SHIFT+'s',saveProjectCB,0},
			{"Load project group",FL_CTRL+'o',loadAllProjectsCB,0},
			{"Load project group (File creatd before 2014-02-23)",0,loadAllProjectsCB,(void*)1},
			{"Save project group",FL_CTRL+'s',saveAllProjectsCB,0},
			{0},
		{"Chunks",0, 0, 0, FL_SUBMENU},
			{"Import sonic 1 chunks",0,ImportS1CBChunks,0},
			{"Import sonic 1 chunks (append)",0,ImportS1CBChunks,(void*)1},
			{"Save chunks as sonic 1 format",0,saveChunkS1CB},
			{0},
		{"Sprites",0, 0, 0, FL_SUBMENU},
			{"Import sprite from image",0,SpriteimportCB,0},
			{"Import sprite from image (append)",0,SpriteimportCB,(void*)1},
			{"Import sprite sheet",0,SpriteSheetimportCB,(void*)1},
			{"Import mapping",0, 0, 0, FL_SUBMENU},
				{"Sonic 1",0,importSonicMappingCB,(void*)tSonic1},
				{"Sonic 2",0,importSonicMappingCB,(void*)tSonic2},
				{"Sonic 3",0,importSonicMappingCB,(void*)tSonic3},
				{0},
			{"Import DPLC",0, 0, 0, FL_SUBMENU},
				{"Sonic 1",0,importSonicDPLCCB,(void*)tSonic1},
				{"Sonic 2 (or sonic 3 character)",0,importSonicDPLCCB,(void*)tSonic2},
				{"Sonic 3",0,importSonicDPLCCB,(void*)tSonic3},
				{0},
			{"Export mapping",0, 0, 0, FL_SUBMENU},
				{"Sonic 1",0,exportSonicMappingCB,(void*)tSonic1},
				{"Sonic 2",0,exportSonicMappingCB,(void*)tSonic2},
				{"Sonic 3",0,exportSonicMappingCB,(void*)tSonic3},
				{0},
			{"Export DPLC",0, 0, 0, FL_SUBMENU},
				{"Sonic 1",0,exportSonicDPLCCB,(void*)tSonic1},
				{"Sonic 2 (or sonic 3 character)",0,exportSonicDPLCCB,(void*)tSonic2},
				{"Sonic 3",0,exportSonicDPLCCB,(void*)tSonic3},
				{0},
			{0},
		{0},
	{"Palette actions",0, 0, 0, FL_SUBMENU},
		{"Generate optimal palette with x amount of colors",0,generate_optimal_palette,0},
		{"Clear entire Palette",0,clearPalette,0},
		{"Pick nearest color algorithm",0,pickNearAlg,0},
		{"RGB color to entry",0,rgb_pal_to_entry,0},
		{"Entry to RGB color",0,entryToRgb,0},
		{"Sort each row by",0,sortRowbyCB,0},
		{0},
	{"Tile actions",0, 0, 0, FL_SUBMENU},
		{"Append blank tile to end of buffer",0,new_tile,0},
		{"Fill tile with selected color",0,fill_tile,0},
		{"Fill tile with color 0",0,blank_tile,0},
		{"Remove duplicate truecolor tiles",0,remove_duplicate_truecolor,0},
		{"Remove duplicate tiles",0,remove_duplicate_tiles,0},
		{"Update dither all tiles",0,update_all_tiles,0},
		{"Delete currently selected tile",0,delete_tile_at_location,0},
		{"Create new tiles for flipped tiles",0,tilesnewfilppedCB},
		{0},
	{"TileMap actions",0, 0, 0, FL_SUBMENU},
		{"Fix tile delete on tilemap",0,tilemap_remove_callback,0},
		{"Toggle TrueColor Viewing (defaults to off)",0,trueColTileToggle,0},
		{"Pick Tile row based on color delta",0,tileDPicker,0},
		{"Auto determine if use shadow highlight",0,shadow_highligh_findout,0},
		{"Dither tilemap as image",0,dither_tilemap_as_imageCB,0},
		{"File tile map with selection including attributes",0,fill_tile_map_with_tile,(void *)0},
		{"Fix out of range tiles (replace with current attributes in plane editor)",0,FixOutOfRangeCB,0},
		{0},
	{"Sprite actions",0, 0, 0, FL_SUBMENU},
		{"Generate optimal palette for selected sprite",0,generate_optimal_palette,(void*)1},
		{"Dither sprite as image",0,ditherSpriteAsImageCB,0},
		{"Dither all sprites as image",0,ditherSpriteAsImageAllCB,0},
		{"Remove blank and duplicate tiles without affect sprite amount",0,optimizeSpritesCB,0},
		{0},
	{"Undo/Redo",0, 0, 0, FL_SUBMENU},
		{"Undo",FL_CTRL+'z',undoCB},
		{"Redo",FL_CTRL+'y',redoCB},
		//{"Show estimated ram usage",0,showMemUsageUndo},
		{"Show history window",FL_CTRL+'h',historyWindow},
		{"Clear undo buffer",0,clearUndoCB},
		{0},
	{"Help",0, 0, 0, FL_SUBMENU},
		{"About",0,showAbout},
		{0},
	{0}
};
static const Fl_Menu_Item ditherChoices[]={
	{"Floyd Steinberg",0,set_ditherAlg,(void *)0},
	{"Riemersma",0,set_ditherAlg,(void *)1},
	{"Nearest Color",0,set_ditherAlg,(void *)2},
	{"Vertical dithering",0, set_ditherAlg,(void *)3},
	{"Yliluoma 1",0,set_ditherAlg,(void *)4},
	{"Yliluoma 2",0,set_ditherAlg,(void *)5},
	{"Yliluoma 3",0,set_ditherAlg,(void *)6},
	{"Thomas Knoll",0,set_ditherAlg,(void *)7},
	{0}
};
static const Fl_Menu_Item subSysNES[]={
	{"1x1 tile palette",0,setSubSysCB,(void*)NES1x1},
	{"2x2 tile palette",0,setSubSysCB,(void*)NES2x2},
	{0}
};
static const Fl_Menu_Item SolidMenu[]={
	{"Not solid",0,solidCB,(void*)0},
	{"Top solid",0,solidCB,(void*)1},
	{"Left/Right/Bottom solid",0,solidCB,(void*)2},
	{"All solid",0,solidCB,(void*)3},
	{0}
};
extern const char * MapWidthTxt;
extern const char * MapHeightTxt;
static const char * TooltipZoom="By changing this slider you are changing the magnification of the tile for example if this slider was set to 10 that would mean that the tile is magnified by a factor of 10";
extern const char*spriteDefName;
extern const char*spritesName;
void editor::_editor(){
	//create the window
	menu = new Fl_Menu_Bar(0,0,800,24);//Create menubar, items..
	menu->copy(menuEditor);
	tile_placer_tile_offset_y=default_tile_placer_tile_offset_y;
	true_color_box_x=default_true_color_box_x;
	true_color_box_y=default_true_color_box_y;
	tile_edit_truecolor_off_x=default_tile_edit_truecolor_off_x;
	tile_edit_truecolor_off_y=default_tile_edit_truecolor_off_y;
	std::fill(tabsHidden,&tabsHidden[shareAmtPj],false);
	{
		the_tabs = new Fl_Tabs(0, 24, 800, 576);
		the_tabs->callback(set_mode_tabs);
		int rx,ry,rw,rh;
		the_tabs->client_area(rx,ry,rw,rh);
		{
			TabsMain[pal_edit] = new Fl_Group(rx, ry, rw, rh, "Palette editor");
			//stuff realed to this group should go here
			palEdit.more_init(4);
			pal_size = new Fl_Hor_Value_Slider(128,384,320,24,"Palette box size");
			pal_size->minimum(1); pal_size->maximum(42);
			pal_size->step(1);
			pal_size->value(32);
			pal_size->align(FL_ALIGN_LEFT);
			pal_size->callback(redrawOnlyCB);
			ditherPower = new Fl_Hor_Value_Slider(128,416,320,24,"Dither Power");
			ditherPower->tooltip("A lower value results in more dithering artifacts a higer value results in less artifacts");
			ditherPower->minimum(1);
			ditherPower->maximum(255);
			ditherPower->step(1);
			ditherPower->value(16);
			ditherPower->align(FL_ALIGN_LEFT);
			{
				shadow_highlight_switch = new Fl_Group(112, 288, 800, 480);
				{
					genSHbtns[0] = new Fl_Round_Button(96, 280, 64, 32, "Normal");
					genSHbtns[0]->type(FL_RADIO_BUTTON);
					genSHbtns[0]->tooltip("This is the regular sega genesis color space. When shadow/highlight mode is disabled all tiles will look like this however when enabling shadow higligh mode and a tile is set to high prioraty you will the tile will use these set of colors");
					genSHbtns[0]->callback((Fl_Callback*) set_palette_type_callback,(void *)0);
					genSHbtns[0]->set();
				} // Fl_Round_Button* o
				{
					genSHbtns[1] = new Fl_Round_Button(164, 280, 64, 32, "Shadow");
					genSHbtns[1]->tooltip("This mode uses the color sets that the vdp uses when shadow highlight mode is enabled by setting bit 3 (the LSB being bit 0) to 1 in the vdp register 0C also for the tile to be shadowed the tile's priority must be set at 0 or low priority");
					genSHbtns[1]->type(FL_RADIO_BUTTON);
					genSHbtns[1]->callback((Fl_Callback*) set_palette_type_callback,(void *)8);
				} // Fl_Round_Button* o
				{
					genSHbtns[2] = new Fl_Round_Button(240, 280, 64, 32, "Highlight");
					genSHbtns[2]->tooltip("This mode uses the color sets that a highlighted sprite or tile uses. To make a tile highlighted use a mask sprite");
					genSHbtns[2]->type(FL_RADIO_BUTTON);
					genSHbtns[2]->callback((Fl_Callback*) set_palette_type_callback,(void *)16);
				} // Fl_Round_Button* o
				shadow_highlight_switch->end();
			}
			{
				Fl_Group *o = new Fl_Group(96, 312, 800, 480);
				{
					GameSys[sega_genesis] = new Fl_Round_Button(96, 312, 96, 32, "Sega Genesis");
					GameSys[sega_genesis]->tooltip("Sets the editing mode to Sega Genesis or Sega Mega Drive");
					GameSys[sega_genesis]->type(FL_RADIO_BUTTON);
					GameSys[sega_genesis]->callback((Fl_Callback*) set_game_system,(void *)sega_genesis);
					GameSys[sega_genesis]->set();
				} // Fl_Round_Button* o
				{
					GameSys[NES] = new Fl_Round_Button(224, 312, 64, 32, "NES");
					GameSys[NES]->tooltip("Sets the editing mode to Nintendo Entertamint System or Famicon");
					GameSys[NES]->type(FL_RADIO_BUTTON);
					GameSys[NES]->callback((Fl_Callback*) set_game_system,(void *)NES);
				} // Fl_Round_Button* o
				o->end();
			} // End of buttons
			{Fl_Choice *o = new Fl_Choice(64, 464, 144, 24);
			o->copy(ditherChoices);}
			subSysC=new Fl_Choice(208, 464, 128, 24);
			subSysC->copy(subSysNES);
			subSysC->value(1);
			subSysC->hide();
			{ Fl_Group *o = new Fl_Group(306, 188, 88, 96);
				{
					palType[0] = new Fl_Round_Button(306, 188, 64, 32, "Free");
					palType[0]->type(FL_RADIO_BUTTON);
					palType[0]->set();
					palType[0]->callback((Fl_Callback*) setPalType,(void *)0);
					palType[0]->tooltip(freeDes);
					palType[1] = new Fl_Round_Button(306, 220, 72, 32, "Locked");
					palType[1]->type(FL_RADIO_BUTTON);
					palType[1]->callback((Fl_Callback*) setPalType,(void *)1);
					palType[1]->tooltip(lockedDes);
					palType[2] = new Fl_Round_Button(306, 252, 88, 32, "Reserved");
					palType[2]->type(FL_RADIO_BUTTON);
					palType[2]->callback((Fl_Callback*) setPalType,(void *)2);
					palType[2]->tooltip(reservedDes);
					o->end();
				} // End of buttons
			}//end of group
      			TabsMain[pal_edit]->end();
		} // Fl_Group* o
		{TabsMain[tile_edit] = new Fl_Group(rx, ry, rw, rh, "Tile editor");
			//stuff realed to this group should go here
			{ Fl_Group* o = new Fl_Group(0, 0, 800, 567);
				palRTE[0] = new Fl_Round_Button(384, default_palette_bar_offset_y+40, 56, 32, "Row 0");
				palRTE[0]->type(FL_RADIO_BUTTON);
				palRTE[0]->set();
				palRTE[0]->callback((Fl_Callback*) set_tile_row,(void *)0);
				palRTE[1] = new Fl_Round_Button(448, default_palette_bar_offset_y+40, 56, 32, "Row 1");
				palRTE[1]->type(FL_RADIO_BUTTON);
				palRTE[1]->callback((Fl_Callback*) set_tile_row,(void *)1);
				palRTE[2] = new Fl_Round_Button(512, default_palette_bar_offset_y+40, 56, 32, "Row 2");
				palRTE[2]->type(FL_RADIO_BUTTON);
				palRTE[2]->callback((Fl_Callback*) set_tile_row,(void *)2);
				palRTE[3] = new Fl_Round_Button(576, default_palette_bar_offset_y+40, 56, 32, "Row 3");
				palRTE[3]->type(FL_RADIO_BUTTON);
				palRTE[3]->callback((Fl_Callback*) set_tile_row,(void *)3);
			o->end();
			} // Fl_Group* o
			{ Fl_Button *o = new Fl_Button(538, default_palette_bar_offset_y, 104, 32, "Append tile");//these button should be inline with the palette bar
				o->tooltip("This will append a blank tile to the tile buffer in the ram.");
				o->callback(new_tile);
			}
			{ Fl_Button *o = new Fl_Button(656, default_palette_bar_offset_y, 140, 32, "Delete selected tile");
				o->tooltip("This button will delete the currently selected tile");
				o->callback(delete_tile_at_location);
				o->labelsize(12);
			}
			{ Fl_Button *o = new Fl_Button(656, default_palette_bar_offset_y+34,140, 32,"Insert after tile");
				o->callback(insertTileCB);
			}
			tileEdit_pal.more_init();
			rgb_red = new Fl_Hor_Value_Slider(64,default_palette_bar_offset_y+136,128,24,"RGB red");
			rgb_red->minimum(0);
			rgb_red->maximum(255);
			rgb_red->step(1);
			rgb_red->value(0);
			rgb_red->align(FL_ALIGN_LEFT);
			rgb_red->callback(update_truecolor,(void *)0);
			rgb_green = new Fl_Hor_Value_Slider(240,default_palette_bar_offset_y+136,128,24,"Green");
			rgb_green->minimum(0);
			rgb_green->maximum(255);
			rgb_green->step(1);
			rgb_green->value(0);
			rgb_green->align(FL_ALIGN_LEFT);
			rgb_green->callback(update_truecolor,(void *)1);
			rgb_blue = new Fl_Hor_Value_Slider(402,default_palette_bar_offset_y+136,128,24,"Blue");
			rgb_blue->minimum(0);
			rgb_blue->maximum(255);
			rgb_blue->step(1);
			rgb_blue->value(0);
			rgb_blue->align(FL_ALIGN_LEFT);
			rgb_blue->callback(update_truecolor,(void *)2);
			rgb_alpha = new Fl_Hor_Value_Slider(576,default_palette_bar_offset_y+136,128,24,"Alpha");
			rgb_alpha->minimum(0);
			rgb_alpha->maximum(255);
			rgb_alpha->step(1);
			rgb_alpha->value(0);
			rgb_alpha->align(FL_ALIGN_LEFT);
			rgb_alpha->callback(update_truecolor,(void *)3);
			{Fl_Group *o = new Fl_Group(304, 96, 88, 96);
				{
					palType[3] = new Fl_Round_Button(304, 96, 64, 32, "Free");
					palType[3]->type(FL_RADIO_BUTTON);
					palType[3]->set();
					palType[3]->callback((Fl_Callback*) setPalType,(void *)0);
					palType[3]->tooltip(freeDes);
					palType[4] = new Fl_Round_Button(304, 128, 72, 32, "Locked");
					palType[4]->type(FL_RADIO_BUTTON);
					palType[4]->callback((Fl_Callback*) setPalType,(void *)1);
					palType[4]->tooltip(lockedDes);
					palType[5] = new Fl_Round_Button(304, 160, 88, 32, "Reserved");
					palType[5]->type(FL_RADIO_BUTTON);
					palType[5]->callback((Fl_Callback*) setPalType,(void *)2);
					palType[5]->tooltip(reservedDes);
					o->end();
				} // End of buttons
			}//end of group
			{ Fl_Check_Button* o = new Fl_Check_Button(694,default_palette_bar_offset_y+68,100,32,"Show grid?");
				o->callback(set_grid);
				o->tooltip("This button Toggles wheater or not you which to see a grid while editing your tiles. A grid can help you see the spacing betwen each pixel.");
			}
			tile_edit_offset_x=default_tile_edit_offset_x;
			tile_edit_offset_y=default_tile_edit_offset_y;
			tile_size = new Fl_Hor_Value_Slider(448,default_palette_bar_offset_y+72,242,24,"Tile zoom");
			tile_size->tooltip(TooltipZoom);
			tile_size->minimum(1);
			tile_size->maximum(64);
			tile_size->step(1);
			tile_size->value(46);
			tile_size->align(FL_ALIGN_LEFT);
			tile_size->callback(update_offset_tile_edit);
			//now for the tile select slider
			tile_select = new Fl_Hor_Value_Slider(480,default_palette_bar_offset_y+104,312,24,"Tile select");
			tile_select->tooltip("This slider selects which tile that you are editing the first tile is zero");
			tile_select->minimum(0);
			tile_select->maximum(0);
			tile_select->step(1);
			tile_select->align(FL_ALIGN_LEFT);
			tile_select->callback(set_tile_current);
			TabsMain[tile_edit]->end();
		}
		{TabsMain[tile_place] = new Fl_Group(rx,ry,rw,rh,"Plane mapping/block editor");
			{Fl_Group* o = new Fl_Group(tile_place_buttons_x_off, 192, 60, 128);
				palRTE[4] = new Fl_Round_Button(tile_place_buttons_x_off, 192, 60, 28, "Row 0");
				palRTE[4]->type(FL_RADIO_BUTTON);
				palRTE[4]->set();
				palRTE[4]->callback((Fl_Callback*) set_tile_row,(void *)0);
				palRTE[5] = new Fl_Round_Button(tile_place_buttons_x_off, 220, 60, 28, "Row 1");
				palRTE[5]->type(FL_RADIO_BUTTON);
				palRTE[5]->callback((Fl_Callback*) set_tile_row,(void *)1);
				palRTE[6] = new Fl_Round_Button(tile_place_buttons_x_off, 248, 60, 28, "Row 2");
				palRTE[6]->type(FL_RADIO_BUTTON);
				palRTE[6]->callback((Fl_Callback*) set_tile_row,(void *)2);
				palRTE[7] = new Fl_Round_Button(tile_place_buttons_x_off, 276, 60, 28, "Row 3");
				palRTE[7]->type(FL_RADIO_BUTTON);
				palRTE[7]->callback((Fl_Callback*) set_tile_row,(void *)3);
				o->end();
			} // Fl_Group* o
			map_w = new Fl_Int_Input(480,default_palette_bar_offset_y+72,312,24,MapWidthTxt);
			map_w->when(FL_WHEN_ENTER_KEY);
			map_w->value("2");
			map_w->align(FL_ALIGN_LEFT);
			map_w->callback(callback_resize_map);
			map_h = new Fl_Int_Input(480,default_palette_bar_offset_y+104,312,24,MapHeightTxt);
			map_h->when(FL_WHEN_ENTER_KEY);
			map_h->value("2");
			map_h->align(FL_ALIGN_LEFT);
			map_h->callback(callback_resize_map);
			map_amt = new Fl_Int_Input(480,default_palette_bar_offset_y+136,312,24,"Blocks");
			map_amt->value("1");
			map_amt->align(FL_ALIGN_LEFT);
			map_amt->callback(blocksAmtCB);
			map_amt->hide();
			map_x_scroll = new Fl_Scrollbar(default_map_off_x-32, default_map_off_y-32, 800-8-default_map_off_x, 24);
			map_x_scroll->value(0,0,0,0);
			map_x_scroll->type(FL_HORIZONTAL);
			map_x_scroll->tooltip("Use this scroll bar to move around the tile map if you are zoomed in and there is not enough room to display the entire tilemap at once. This scroll bar will move the map left and right.");
			map_x_scroll->callback(update_map_scroll_x);
			map_x_scroll->hide();
			map_x_scroll->linesize(1);
			map_y_scroll = new Fl_Scrollbar(default_map_off_x-32, default_map_off_y, 24, 600-8-default_map_off_y);
			map_y_scroll->value(0,0,0,0);
			//map_x_scroll->type(FL_HORIZONTAL);
			map_y_scroll->tooltip("Use this scroll bar to move around the tile map if you are zoomed in and there is not enough room to display the entire tilemap at once. This scroll bar will move the map up and down.");
			map_y_scroll->callback(update_map_scroll_y);
			map_y_scroll->hide();
			map_y_scroll->linesize(1);
			//now for the tile select slider
			tile_select_2 = new Fl_Hor_Value_Slider(480,default_palette_bar_offset_y+40,312,24,"Tile select");
			tile_select_2->tooltip("This slider allows you to choice which tile you would like to place on the map remember you can both horizontally and vertically flip the tile once placed on the map and select which row the tile uses");
			tile_select_2->minimum(0);
			tile_select_2->maximum(0);
			tile_select_2->step(1);
			tile_select_2->align(FL_ALIGN_LEFT);
			tile_select_2->callback(set_tile_currentTP);
			totalTiles=new Fl_Box(536,default_palette_bar_offset_y,128,64);
			totalTiles->labelsize(12);
			tileMap_pal.more_init();
			//buttons for tile settings
			{ Fl_Group *o = new Fl_Group(304, 96, 88, 96);
				{
					palType[6] = new Fl_Round_Button(304, 96, 64, 32, "Free");
					palType[6]->type(FL_RADIO_BUTTON);
					palType[6]->set();
					palType[6]->callback((Fl_Callback*) setPalType,(void *)0);
					palType[6]->tooltip(freeDes);
					palType[7] = new Fl_Round_Button(304, 128, 72, 32, "Locked");
					palType[7]->type(FL_RADIO_BUTTON);
					palType[7]->callback((Fl_Callback*) setPalType,(void *)1);
					palType[7]->tooltip(lockedDes);
					palType[8] = new Fl_Round_Button(304, 160, 88, 32, "Reserved");
					palType[8]->type(FL_RADIO_BUTTON);
					palType[8]->callback((Fl_Callback*) setPalType,(void *)2);
					palType[8]->tooltip(reservedDes);
					o->end();
				} // End of buttons
			}//end of group
			
			hflipCB[0] = new Fl_Check_Button(tile_place_buttons_x_off,304,64,32,"Hflip");
			hflipCB[0]->callback(set_hflipCB,(void*)0);
			hflipCB[0]->tooltip("This sets whether or not the tile is flipped horizontally");
			vflipCB[0] = new Fl_Check_Button(tile_place_buttons_x_off,336,64,32,"Vflip");
			vflipCB[0]->callback(set_vflipCB,(void*)0);
			vflipCB[0]->tooltip("This sets whether or not the tile is flipped vertically");
			prioCB[0] = new Fl_Check_Button(tile_place_buttons_x_off,368,72,32,"Priority");
			prioCB[0]->callback(set_prioCB,(void*)0);
			prioCB[0]->tooltip("If checked tile is high priority");
			{ Fl_Check_Button* o = new Fl_Check_Button(tile_place_buttons_x_off,400,96,32,"Show grid?");
				o->callback(set_grid_placer);
				o->tooltip("This button toggles whether or not a grid is visible over the tilemap this will allow you to easily see were each tile is");
			}
			BlocksCBtn = new Fl_Check_Button(tile_place_buttons_x_off,432,96,32,"Blocks?");
			BlocksCBtn->callback(toggleBlocksCB);
			BlocksCBtn->tooltip("Toggles if tilemap is treated as blocks");
			{ Fl_Check_Button* o = new Fl_Check_Button(tile_place_buttons_x_off,464,192,32,"Show only selected row");
				o->callback(toggleRowSolo);
				o->tooltip("When checked tiles that do not use the selected row will not be drawn");
			}
			place_tile_size = new Fl_Hor_Value_Slider(tile_place_buttons_x_off,512,168,24,"Tile zoom factor:");
			place_tile_size->minimum(1);
			place_tile_size->maximum(16);
			place_tile_size->step(1);
			place_tile_size->value(12);
			place_tile_size->align(FL_ALIGN_TOP);
			place_tile_size->callback(update_map_size);
			place_tile_size->tooltip(TooltipZoom);

			tmapOffset = new Fl_Int_Input(tile_place_buttons_x_off,552,168,24,"Tile offset");
			tmapOffset->when(FL_WHEN_ENTER_KEY);
			tmapOffset->value("0");
			tmapOffset->align(FL_ALIGN_TOP);
			tmapOffset->callback(setTmapOffsetCB);

			cordDisp[0]=new Fl_Box(tile_place_buttons_x_off,556,128,64);
			cordDisp[0]->labelsize(12);

			TabsMain[tile_place]->end();
		}
		{TabsMain[chunkEditor] = new Fl_Group(rx,ry,rw,rh,"Chunk editor");
			useBlocksChunkCBtn=new Fl_Check_Button(8, 48, 152, 24, "Use blocks");
			useBlocksChunkCBtn->callback(useBlocksCB);
			chunk_tile_size = new Fl_Hor_Value_Slider(tile_place_buttons_x_off,512,160,24,"Tile zoom factor:");
			chunk_tile_size->minimum(1);
			chunk_tile_size->maximum(16);
			chunk_tile_size->step(1);
			chunk_tile_size->value(2);
			chunk_tile_size->align(FL_ALIGN_TOP);
			chunk_tile_size->callback(scrollChunkCB);
			chunk_tile_size->tooltip(TooltipZoom);
			
			chunkX = new Fl_Scrollbar(DefaultChunkX-32, DefaultChunkY-32, 800-DefaultChunkX+24, 24);
			chunkX->value(0,0,0,0);
			chunkX->type(FL_HORIZONTAL);
			chunkX->callback(scrollChunkX);
			chunkX->hide();
			
			chunkY = new Fl_Scrollbar(DefaultChunkX-32, DefaultChunkY, 24, 600-8-DefaultChunkY);
			chunkY->value(0,0,0,0);
			chunkY->callback(scrollChunkY);
			chunkY->hide();
			
			chunk_select = new Fl_Hor_Value_Slider(tile_place_buttons_x_off,88,160,24,"Chunk select");
			chunk_select->minimum(0);
			chunk_select->maximum(0);
			chunk_select->step(1);
			chunk_select->value(0);
			chunk_select->align(FL_ALIGN_TOP);
			chunk_select->callback(currentChunkCB);
			
			tile_select_3 = new Fl_Hor_Value_Slider(tile_place_buttons_x_off,136,160,24,"Tile select");
			tile_select_3->minimum(0);
			tile_select_3->maximum(0);
			tile_select_3->step(1);
			tile_select_3->align(FL_ALIGN_TOP);
			tile_select_3->callback(selBlockCB);

			hflipCB[1] = new Fl_Check_Button(tile_place_buttons_x_off,160,64,32,"Hflip");
			hflipCB[1]->callback(set_hflipCB,(void*)1);
			vflipCB[1] = new Fl_Check_Button(tile_place_buttons_x_off,192,64,32,"Vflip");
			vflipCB[1]->callback(set_vflipCB,(void*)1);
			prioCB[1] = new Fl_Check_Button(tile_place_buttons_x_off,224,72,32,"Priority");
			prioCB[1]->callback(set_prioCB,(void*)1);

			solidChunkMenu=new Fl_Choice(tile_place_buttons_x_off,256,128,24);
			solidChunkMenu->copy(SolidMenu);

			chunksize[0]=new Fl_Int_Input(tile_place_buttons_x_off,296,128,24,"Width (in tiles)");
			chunksize[0]->when(FL_WHEN_ENTER_KEY);
			chunksize[0]->align(FL_ALIGN_TOP);
			chunksize[0]->callback(resizeChunkCB);
			chunksize[0]->value("16");

			chunksize[1]=new Fl_Int_Input(tile_place_buttons_x_off,336,128,24,"Height (in tiles)");
			chunksize[1]->when(FL_WHEN_ENTER_KEY);
			chunksize[1]->align(FL_ALIGN_TOP);
			chunksize[1]->callback(resizeChunkCB);
			chunksize[1]->value("16");

			cordDisp[1]=new Fl_Box(tile_place_buttons_x_off,556,128,64);
			cordDisp[1]->labelsize(12);

			{Fl_Button *o = new Fl_Button(tile_place_buttons_x_off,364, 112, 32, "Append chunk");
				o->callback(appendChunkCB);
			}

			{Fl_Button *o = new Fl_Button(tile_place_buttons_x_off,396, 128, 32, "Insert after chunk");
				o->callback(insertChunkCB);
			}
			{Fl_Button *o = new Fl_Button(tile_place_buttons_x_off,428, 160, 32, "Delete selected chunk");
				o->callback(delChunkAtCB);
			}
			updateChunkSize();

			TabsMain[chunkEditor]->end();
		}
		{TabsMain[spriteEditor] = new Fl_Group(rx,ry,rw,rh,"Sprites");
			spritePal.more_init(1,16,54,true,128,true);

			spritegrouptxt = new Fl_Input(tile_place_buttons_x_off+616,56,168,24,"Group name");
			spritegrouptxt->value(spriteDefName);
			spritegrouptxt->callback(assignSpritegroupnameCB);

			spriteselgroup=new Fl_Hor_Value_Slider(tile_place_buttons_x_off,184,168,22,"Sprite group select:");
			spriteselgroup->step(1);
			spriteselgroup->maximum(0);
			spriteselgroup->align(FL_ALIGN_TOP);
			spriteselgroup->callback(selspriteGroup);
			spriteselgroup->labelsize(12);

			spritesel=new Fl_Hor_Value_Slider(tile_place_buttons_x_off,220,168,22,"Sprite select:");
			spritesel->step(1);
			spritesel->maximum(0);
			spritesel->align(FL_ALIGN_TOP); spritesel->callback(selSpriteCB);
			spritesel->labelsize(12);

			spritest=new Fl_Hor_Value_Slider(tile_place_buttons_x_off,256,168,22,"Start tile:");
			spritest->step(1);
			spritest->maximum(0);
			spritest->align(FL_ALIGN_TOP);
			spritest->callback(setvalueSpriteCB,0);
			spritest->labelsize(12);

			spriteslat=new Fl_Hor_Value_Slider(tile_place_buttons_x_off,292,168,22,"Mapping tile");
			spriteslat->step(1);
			spriteslat->maximum(0);
			spriteslat->align(FL_ALIGN_TOP);
			spriteslat->callback(setvalueSpriteCB,(void*)4);
			spriteslat->labelsize(12);

			spritesize[0]=new Fl_Hor_Value_Slider(tile_place_buttons_x_off+40,316,128,22,"Width");
			spritesize[0]->step(1);
			spritesize[0]->value(1);
			spritesize[0]->minimum(1);
			spritesize[0]->maximum(4);
			spritesize[0]->align(FL_ALIGN_LEFT);
			spritesize[0]->callback(setvalueSpriteCB,(void*)1);
			spritesize[0]->labelsize(12);

			spritesize[1]=new Fl_Hor_Value_Slider(tile_place_buttons_x_off+40,340,128,22,"Height");
			spritesize[1]->step(1);
			spritesize[1]->value(1);
			spritesize[1]->minimum(1);
			spritesize[1]->maximum(4);
			spritesize[1]->align(FL_ALIGN_LEFT);
			spritesize[1]->callback(setvalueSpriteCB,(void*)2);
			spritesize[1]->labelsize(12);

			spritepalrow=new Fl_Hor_Value_Slider(tile_place_buttons_x_off,376,168,22,"Palette row:");
			spritepalrow->step(1);
			spritepalrow->maximum(3);
			spritepalrow->align(FL_ALIGN_TOP);
			spritepalrow->callback(setvalueSpriteCB,(void*)3);
			spritepalrow->labelsize(12);

			spritezoom=new Fl_Hor_Value_Slider(tile_place_buttons_x_off+38,400,130,22,"Zoom");
			spritezoom->step(1);
			spritezoom->minimum(1);
			spritezoom->value(16);
			spritezoom->maximum(16);
			spritezoom->align(FL_ALIGN_LEFT);
			spritezoom->callback(redrawOnlyCB);
			spritezoom->labelsize(12);

			spritesoff[0] = new Fl_Int_Input(tile_place_buttons_x_off+62,424,106,24,"Offset X:");
			spritesoff[0]->when(FL_WHEN_ENTER_KEY);
			spritesoff[0]->value("0");
			spritesoff[0]->align(FL_ALIGN_LEFT);
			spritesoff[0]->callback(setoffspriteCB,0);
			
			spritesoff[1] = new Fl_Int_Input(tile_place_buttons_x_off+62,448,106,24,"Offset Y:");
			spritesoff[1]->when(FL_WHEN_ENTER_KEY);
			spritesoff[1]->value("0");
			spritesoff[1]->align(FL_ALIGN_LEFT);
			spritesoff[1]->callback(setoffspriteCB,(void*)1);
			
			spritehflip = new Fl_Check_Button(tile_place_buttons_x_off,470,48,20,"Hflip");
			spritehflip->callback(spriteHflipCB);
			spritevflip = new Fl_Check_Button(tile_place_buttons_x_off+52,470,48,20,"Vflip");
			spritevflip->callback(spriteVflipCB);
			spriteprio = new Fl_Check_Button(tile_place_buttons_x_off+104,470,56,20,"Priority");
			spriteprio->callback(spritePrioCB);

			{Fl_Button *o = new Fl_Button(tile_place_buttons_x_off, 492, 64, 28, "Append");
			o->callback(appendSpriteCB,0);
			o->labelsize(12);}
			{Fl_Button *o = new Fl_Button(tile_place_buttons_x_off+72, 492, 96, 28, "Append group");
			o->callback(appendSpriteCB,(void*)1);
			o->labelsize(12);}
			{Fl_Button *o = new Fl_Button(tile_place_buttons_x_off, 522, 64, 28, "Delete");
			o->callback(delSpriteCB,0);
			o->labelsize(12);}
			{Fl_Button *o = new Fl_Button(tile_place_buttons_x_off+72, 522, 96, 28, "Delete group");
			o->callback(delSpriteCB,(void*)1);
			o->labelsize(12);}
			spritealign[0] = new Fl_Button(tile_place_buttons_x_off, 552, 32, 28, "Left");
			spritealign[0]->labelsize(12);
			spritealign[0]->callback(alignSpriteCB,(void*)0);

			spritealign[1] = new Fl_Button(tile_place_buttons_x_off+34, 552, 40, 28, "Right");
			spritealign[1]->labelsize(12);
			spritealign[1]->callback(alignSpriteCB,(void*)1);

			spritealign[2] = new Fl_Button(tile_place_buttons_x_off+78, 552, 28, 28, "Top");
			spritealign[2]->labelsize(12);
			spritealign[2]->callback(alignSpriteCB,(void*)2);

			spritealign[3] = new Fl_Button(tile_place_buttons_x_off+112, 552, 48, 28, "Bottom");
			spritealign[3]->labelsize(12);
			spritealign[3]->callback(alignSpriteCB,(void*)3);


			{
				Fl_Group *o = new Fl_Group(tile_place_buttons_x_off, 572, 800, 480);
				{
					Fl_Round_Button*m = new Fl_Round_Button(tile_place_buttons_x_off, 572, 96, 32, "Top corner");
					m->type(FL_RADIO_BUTTON);
					m->callback(setDrawSpriteCB,(void *)0);
					m->set();
				} // Fl_Round_Button* o
				{
					Fl_Round_Button*m = new Fl_Round_Button(tile_place_buttons_x_off+96, 572, 64, 32, "Center");
					m->type(FL_RADIO_BUTTON);
					m->callback(setDrawSpriteCB,(void *)1);
				} // Fl_Round_Button* o
				o->end();
			} // End of buttons

			TabsMain[spriteEditor]->end();
		}
		{TabsMain[levelEditor] = new Fl_Group(rx,ry,rw,rh,"Level editor");
			TabsMain[levelEditor]->end();
		}
		{TabsMain[settingsTab] = new Fl_Group(rx,ry,rw,rh,"Settings/projects");
			projectSelect=new Fl_Hor_Value_Slider(112,56,128,24,"Current project");
			projectSelect->minimum(0);
			projectSelect->maximum(0);
			projectSelect->step(1);
			projectSelect->value(0);
			projectSelect->align(FL_ALIGN_LEFT);
			projectSelect->callback(switchProjectCB);
			{Fl_Button *o = new Fl_Button(260, 52, 152, 32, "Append blank project");
				o->callback(appendProjectCB);
			}
			{Fl_Button *o = new Fl_Button(428, 52, 168, 32, "Delete selected project");
				o->callback(deleteProjectCB);
			}
			//IMPORTANT if adding a new tab remember to update these
			sharePrj[0]=new Fl_Check_Button(8,112,112,16,"Share palette");
			sharePrj[0]->callback(shareProjectCB,(void*)pjHavePal);
			sharePrj[1]=new Fl_Check_Button(120,112,96,16,"Share Tiles");
			sharePrj[1]->callback(shareProjectCB,(void*)pjHaveTiles);
			sharePrj[2]=new Fl_Check_Button(216,112,120,16,"Share TileMap");
			sharePrj[2]->callback(shareProjectCB,(void*)pjHaveMap);
			sharePrj[3]=new Fl_Check_Button(336,112,120,16,"Share chunks");
			sharePrj[3]->callback(shareProjectCB,(void*)pjHaveChunks);
			sharePrj[4]=new Fl_Check_Button(456,112,120,16,"Share sprites");
			sharePrj[4]->callback(shareProjectCB,(void*)pjHaveSprites);
			sharePrj[5]=new Fl_Check_Button(576,112,120,16,"Share level");
			sharePrj[5]->callback(shareProjectCB,(void*)pjHaveLevel);
			
			havePrj[0]=new Fl_Check_Button(8,88,112,16,"Have palette");
			havePrj[0]->callback(haveCB,(void*)pjHavePal);
			havePrj[1]=new Fl_Check_Button(120,88,96,16,"Have tiles");
			havePrj[1]->callback(haveCB,(void*)pjHaveTiles);
			havePrj[2]=new Fl_Check_Button(232,88,120,16,"Have tileMap");
			havePrj[2]->callback(haveCB,(void*)pjHaveMap);
			havePrj[3]=new Fl_Check_Button(344,88,120,16,"Have chunks");
			havePrj[3]->callback(haveCB,(void*)pjHaveChunks);
			havePrj[4]=new Fl_Check_Button(456,88,120,16,"Have sprites");
			havePrj[4]->callback(haveCB,(void*)pjHaveSprites);
			havePrj[5]=new Fl_Check_Button(568,88,120,16,"Have level");
			havePrj[5]->callback(haveCB,(void*)pjHaveLevel);
			
			shareWith[0]=new Fl_Hor_Value_Slider(8,142,128,24,"Share palette with:");
			shareWith[0]->callback(switchShareCB,(void*)pjHavePal);
			shareWith[1]=new Fl_Hor_Value_Slider(136,142,128,24,"Share tiles with:");
			shareWith[1]->callback(switchShareCB,(void*)pjHaveTiles);
			shareWith[2]=new Fl_Hor_Value_Slider(264,142,128,24,"Share tileMap with:");
			shareWith[2]->callback(switchShareCB,(void*)pjHaveMap);
			shareWith[3]=new Fl_Hor_Value_Slider(400,142,128,24,"Share chunks with:");
			shareWith[3]->callback(switchShareCB,(void*)pjHaveChunks);
			shareWith[4]=new Fl_Hor_Value_Slider(536,142,128,24,"Share sprites with:");
			shareWith[4]->callback(switchShareCB,(void*)pjHaveSprites);
			shareWith[5]=new Fl_Hor_Value_Slider(672,142,128,24,"Share level with:");
			shareWith[5]->callback(switchShareCB,(void*)pjHaveLevel);
			for(unsigned x=0;x<shareAmtPj;++x){
				havePrj[x]->value(1);
				shareWith[x]->minimum(0);
				shareWith[x]->maximum(0);
				shareWith[x]->step(1);
				shareWith[x]->value(0);
				shareWith[x]->align(FL_ALIGN_TOP);
			}
			
			spriteglobaltxt = new Fl_Input(tile_place_buttons_x_off+616,64,168,24,"Global sprites name");
			spriteglobaltxt->value(spritesName);
			spriteglobaltxt->callback(assignSpriteglobalnameCB);
			spriteglobaltxt->align(FL_ALIGN_TOP);

			TxtBufProject = new Fl_Text_Buffer;
			TxtEditProject = new Fl_Text_Editor(8, 184, 640, 370,"Description/Notes");
			TxtEditProject->buffer(TxtBufProject);
			TxtEditProject->textfont(FL_TIMES);
			TxtBufProject->text(currentProject->Name.c_str());
			TabsMain[settingsTab]->end();
		}
	}
}
