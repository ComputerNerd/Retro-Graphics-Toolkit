#include "global.h"
#include "callbacks_palette.h"
#include "callback_tiles.h"
#include "tiles_io.h"
#include "callback_tilemap.h"
#include "callback_gui.h"
#include "callback_project.h"
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
void set_mode_tabs(Fl_Widget* o, void*){
	Fl_Group * val=(Fl_Group*)(Fl_Tabs*)window->the_tabs->value();
	if (val==window->TabsMain[0]){
		mode_editor=pal_edit;
		palEdit.updateSlider();
	}else if (val==window->TabsMain[1]){
		mode_editor=tile_edit;
		tileEdit_pal.updateSlider();
	}else if (val==window->TabsMain[2]){
		mode_editor=tile_place;
		tileMap_pal.updateSlider();
	}else if(val==window->TabsMain[3]){
		mode_editor=chunckEditor;
	}else if (val==window->TabsMain[4]){
		mode_editor=settingsTab;
	}
}
static const char * freeDes="This sets the currently selected palette entry to free meaning that this color can be changed";
static const char * lockedDes="This sets the currently selected palette entry to locked meaning that this color cannot be changed but tiles can still use it";
static const char * reservedDes="This sets the currently selected palette entry to reserved meaning that this color cannot be changed or used in tiles note that you may need make sure all tiles get re-dithered to ensure that this rule is enforced";
static const Fl_Menu_Item menuEditor[]={
	{"File",0, 0, 0, FL_SUBMENU},
		{"Open palette",0,loadPalette,0},
		{"Open tiles",0,load_tiles,0},
		{"Open Truecolor Tiles",0,load_truecolor_tiles,0},
		{"Append tiles",0,load_tiles,(void*)1},
		{"Open tile map or blocks and if NES attributes",0,load_tile_map,0},
		{"Import image to tilemap",0,load_image_to_tilemap,0},
		{"Save tilemap as image",0,save_tilemap_as_image,0},
		{"Save tilemap as with system color space",0,save_tilemap_as_colspace,0},
		{"Save Palette",0, save_palette,0},
		{"Save tiles",0,save_tiles,0},
		{"Save truecolor tiles",0,save_tiles_truecolor,0},
		{"Save tile map and if nes attributes",0,save_map,0},
		{"Load project",0,loadProjectCB,0},
		{"Save project",0,saveProjectCB,0},
		{"Load project group",0,loadAllProjectsCB,0},
		{"Save project group",0,saveAllProjectsCB,0},
		{0},
	{"Palette Actions",0, 0, 0, FL_SUBMENU},
		{"Generate optimal palette with x amount of colors",0,generate_optimal_palette,0},
		{"Clear entire Palette",0,clearPalette,0},
		{"Pick nearest color algorithm",0,pickNearAlg,0},
		{0},
	{"Tile Actions",0, 0, 0, FL_SUBMENU},
		{"Append blank tile to end of buffer",0,new_tile,0},
		{"Fill tile with selected color",0,fill_tile,0},
		{"Fill tile with color 0",0,blank_tile,0},
		{"Remove duplicate truecolor tiles",0,remove_duplicate_truecolor,0},
		{"Remove duplicate tiles",0,remove_duplicate_tiles,0},
		{"Update dither all tiles",0,update_all_tiles,0},
		{"Delete currently selected tile",0,delete_tile_at_location,0},
		{0},
	{"TileMap Actions",0, 0, 0, FL_SUBMENU},
		{"Remove tile from tilemap",0,tilemap_remove_callback,0},
		{"Toggle TrueColor Viewing (defaults to off)",0,trueColTileToggle,0},
		{"Pick Tile row based on delta",0,tileDPicker,0},
		{"Auto determine if use shadow highlight",0,shadow_highligh_findout,0},
		{"Dither tilemap as image",0,dither_tilemap_as_image,0},
		{"File tile map with selection includeing attributes",0,fill_tile_map_with_tile,(void *)0},
		{"Fix out of range tiles (replace with current attributes in plane editor)",0,FixOutOfRangeCB,0},
		{0},
	{"Help",0, 0, 0, FL_SUBMENU},
		{"About",0,showAbout},
		{0},
	{0}
};
void editor::_editor(){
	//create the window
	menu = new Fl_Menu_Bar(0,0,800,24);//Create menubar, items..
	menu->copy(menuEditor);
	tile_placer_tile_offset_y=default_tile_placer_tile_offset_y;
	true_color_box_x=default_true_color_box_x;
	true_color_box_y=default_true_color_box_y;
	tile_edit_truecolor_off_x=default_tile_edit_truecolor_off_x;
	tile_edit_truecolor_off_y=default_tile_edit_truecolor_off_y;
	{
		the_tabs = new Fl_Tabs(0, 24, 800, 576);
		the_tabs->callback(set_mode_tabs);
		int rx,ry,rw,rh;
		the_tabs->client_area(rx,ry,rw,rh);
		{
			TabsMain[0] = new Fl_Group(rx, ry, rw, rh, "palette editor");
			//stuff realed to this group should go here
			palEdit.more_init(4);
			pal_size = new Fl_Hor_Value_Slider(128,384,320,24,"Palette box size");
			pal_size->minimum(1); pal_size->maximum(42);
			pal_size->step(1);
			pal_size->value(32);
			pal_size->align(FL_ALIGN_LEFT);
			pal_size->callback(update_box_size);
			ditherPower = new Fl_Hor_Value_Slider(128,416,320,24,"Dither Power");
			ditherPower->tooltip("A lower value resualts in more dithering artifacts a higer value resualts in less artifacts");
			ditherPower->minimum(1); ditherPower->maximum(255);
			ditherPower->step(1);
			ditherPower->value(16);
			ditherPower->align(FL_ALIGN_LEFT);
			{
				shadow_highlight_switch = new Fl_Group(112, 288, 800, 480);
				{
					Fl_Round_Button* o = new Fl_Round_Button(96, 280, 64, 32, "Normal");
					o->type(FL_RADIO_BUTTON);
					o->tooltip("This is the default sega genesis color.When shadow/highlight mode is disabled all tiles will look like this however when enabling shadow higligh mode and a tile is set to high prioraty you will the tile will use these set of colors");
					o->callback((Fl_Callback*) set_palette_type_callback,(void *)0);
					o->set();
				} // Fl_Round_Button* o
				{
					Fl_Round_Button* o = new Fl_Round_Button(164, 280, 64, 32, "Shadow");
					o->tooltip("This mode uses the color sets that the vdp uses when shadow highlight mode is enabled by setting bit 3 (the LSB being bit 0) to 1 in the vdp register 0C also for the tile to be shadowed the tile's priority must be set at 0 or low priority");
					o->type(FL_RADIO_BUTTON);
					o->callback((Fl_Callback*) set_palette_type_callback,(void *)8);
				} // Fl_Round_Button* o
				{
					Fl_Round_Button* o = new Fl_Round_Button(240, 280, 64, 32, "Highlight");
					o->tooltip("This mode uses the color sets that a highlighted sprite or tile uses to make a tile highlighted use a mask sprite");
					o->type(FL_RADIO_BUTTON);
					o->callback((Fl_Callback*) set_palette_type_callback,(void *)16);
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
			{
				Fl_Group *o = new Fl_Group(0, 0, 800, 500);
				{
					Fl_Round_Button* o = new Fl_Round_Button(64, 440, 128, 32, "Floyd Steinberg");
					o->tooltip("Common algorithm. Very simple but effective\nSee: https://en.wikipedia.org/wiki/Floyd%E2%80%93Steinberg_dithering");
					o->type(FL_RADIO_BUTTON);
					o->set();
					o->callback((Fl_Callback*) set_ditherAlg,(void *)0);
				} // Fl_Round_Button* o
				{
					Fl_Round_Button* o = new Fl_Round_Button(196, 440, 104, 32, "Riemersma");
					o->tooltip("A Balanced Dithering Technique\nBy Thiadmer Riemersma, December 01, 1998\nFrom: http://www.drdobbs.com/a-balanced-dithering-technique/184403590");
					o->type(FL_RADIO_BUTTON);
					o->callback((Fl_Callback*) set_ditherAlg,(void *)1);
				} // Fl_Round_Button* o
				{
					Fl_Round_Button* o = new Fl_Round_Button(296, 440, 120, 32, "Nearest Color");
					o->tooltip("No error diffusion/dithering is used as the name says just picks the nearest color");
					o->type(FL_RADIO_BUTTON);
					o->callback((Fl_Callback*) set_ditherAlg,(void *)2);
				} // Fl_Round_Button* o
				{
					Fl_Round_Button* o = new Fl_Round_Button(420, 440, 120, 32, "Yliluoma 3");
					o->tooltip("Yliluoma's ordered dithering algorithm 3 credits go to:\nhttp://bisqwit.iki.fi/story/howto/dither/jy/");
					o->type(FL_RADIO_BUTTON);
					o->callback((Fl_Callback*) set_ditherAlg,(void *)3);
				} // Fl_Round_Button* o
				{
					Fl_Round_Button* o = new Fl_Round_Button(548, 440, 120, 32, "Vertical dithering");
					o->tooltip("As seen on the sega genesis a lot");
					o->type(FL_RADIO_BUTTON);
					o->callback((Fl_Callback*) set_ditherAlg,(void *)4);
				} // Fl_Round_Button* o
				o->end();
			} // End of buttons
			{ Fl_Group *o = new Fl_Group(304, 192, 88, 96);
				{
					palType[0] = new Fl_Round_Button(304, 192, 64, 32, "Free");
					palType[0]->type(FL_RADIO_BUTTON);
					palType[0]->set();
					palType[0]->callback((Fl_Callback*) setPalType,(void *)0);
					palType[0]->tooltip(freeDes);
					palType[1] = new Fl_Round_Button(304, 224, 72, 32, "Locked");
					palType[1]->type(FL_RADIO_BUTTON);
					palType[1]->callback((Fl_Callback*) setPalType,(void *)1);
					palType[1]->tooltip(lockedDes);
					palType[2] = new Fl_Round_Button(304, 256, 88, 32, "Reserved");
					palType[2]->type(FL_RADIO_BUTTON);
					palType[2]->callback((Fl_Callback*) setPalType,(void *)2);
					palType[2]->tooltip(reservedDes);
					o->end();
				} // End of buttons
			}//end of group
      			TabsMain[0]->end();
		} // Fl_Group* o
		{TabsMain[1] = new Fl_Group(rx, ry, rw, rh, "Tile Editor");
			//stuff realed to this group should go here
			{ Fl_Group* o = new Fl_Group(0, 0, 800, 567);
				{
					Fl_Round_Button* o = new Fl_Round_Button(384, default_palette_bar_offset_y+40, 56, 32, "Row 0");
					o->type(FL_RADIO_BUTTON);
					o->set();
					o->callback((Fl_Callback*) set_tile_row,(void *)0);
				} // Fl_Round_Button* o
				{
					Fl_Round_Button* o = new Fl_Round_Button(448, default_palette_bar_offset_y+40, 56, 32, "Row 1");
					o->type(FL_RADIO_BUTTON);
					o->callback((Fl_Callback*) set_tile_row,(void *)1);
				} // Fl_Round_Button* o
				{
					Fl_Round_Button* o = new Fl_Round_Button(512, default_palette_bar_offset_y+40, 56, 32, "Row 2");
					o->type(FL_RADIO_BUTTON);
					o->callback((Fl_Callback*) set_tile_row,(void *)2);
				} // Fl_Round_Button* o
				{
					Fl_Round_Button* o = new Fl_Round_Button(576, default_palette_bar_offset_y+40, 56, 32, "Row 3");
					o->type(FL_RADIO_BUTTON);
					o->callback((Fl_Callback*) set_tile_row,(void *)3);
				} // Fl_Round_Button* o
			o->end();
			} // Fl_Group* o
			{ Fl_Check_Button* o = new Fl_Check_Button(640,default_palette_bar_offset_y+40,120,32,"Show grid?");
				o->callback(set_grid);
				o->tooltip("This button Toggles wheater or not you which to see a grid while editing your tiles. A grid can help you see the spacing betwen each pixel.");
			}
			{ Fl_Button *o = new Fl_Button(540, default_palette_bar_offset_y, 120, 32, "New Tile");//these button should be inline with the palette bar
				o->tooltip("This will append a blank tile to the tile buffer in the ram.");
				o->callback(new_tile);
			}
			{ Fl_Button *o = new Fl_Button(668, default_palette_bar_offset_y, 128, 32, "Delete Selected Tile");
				o->tooltip("This button will delete the curretly selected tile");
				o->callback(delete_tile_at_location);
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
			{ Fl_Group *o = new Fl_Group(304, 96, 88, 96);
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
			tile_edit_offset_x=default_tile_edit_offset_x;
			tile_edit_offset_y=default_tile_edit_offset_y;
			tile_size = new Fl_Hor_Value_Slider(496,default_palette_bar_offset_y+72,304,24,"Tile Zoom Factor:");
			tile_size->tooltip("This slider sets magnification a value of 10 would mean the image is being displayed 10 times larger");
			tile_size->minimum(1);
			tile_size->maximum(64);
			tile_size->step(1);
			tile_size->value(46);
			tile_size->align(FL_ALIGN_LEFT);
			tile_size->callback(update_offset_tile_edit);
			//now for the tile select slider
			tile_select = new Fl_Hor_Value_Slider(480,default_palette_bar_offset_y+104,320,24,"Tile Select");
			tile_select->tooltip("This slider selects which tile that you are editing the first tile is zero");
			tile_select->minimum(0);
			tile_select->maximum(0);
			tile_select->step(1);
			tile_select->value(0);
			tile_select->align(FL_ALIGN_LEFT);
			tile_select->callback(set_tile_current);
			TabsMain[1]->end();
		}
		{TabsMain[2] = new Fl_Group(rx,ry,rw,rh,"Plane Mapping/Block Editor");
			{
				Fl_Group* o = new Fl_Group(tile_place_buttons_x_off, 208, 60, 128);
				{
					palRTE[0] = new Fl_Round_Button(tile_place_buttons_x_off, 208, 60, 32, "Row 0");
					palRTE[0]->type(FL_RADIO_BUTTON);
					palRTE[0]->set();
					palRTE[0]->callback((Fl_Callback*) set_tile_row,(void *)0);
				} // Fl_Round_Button* o
				{
					palRTE[1] = new Fl_Round_Button(tile_place_buttons_x_off, 240, 60, 32, "Row 1");
					palRTE[1]->type(FL_RADIO_BUTTON);
					palRTE[1]->callback((Fl_Callback*) set_tile_row,(void *)1);
				} // Fl_Round_Button* o
				{
					palRTE[2] = new Fl_Round_Button(tile_place_buttons_x_off, 272, 60, 32, "Row 2");
					palRTE[2]->type(FL_RADIO_BUTTON);
					palRTE[2]->callback((Fl_Callback*) set_tile_row,(void *)2);
				} // Fl_Round_Button* o
				{
					palRTE[3] = new Fl_Round_Button(tile_place_buttons_x_off, 304, 60, 32, "Row 3");
					palRTE[3]->type(FL_RADIO_BUTTON);
					palRTE[3]->callback((Fl_Callback*) set_tile_row,(void *)3);
				} // Fl_Round_Button* o
				o->end();
			} // Fl_Group* o
			map_w = new Fl_Hor_Value_Slider(480,default_palette_bar_offset_y+72,312,24,"Map width");
			map_w->minimum(1);
			map_w->maximum(0xFFFF);
			map_w->step(1);
			map_w->value(2);
			map_w->align(FL_ALIGN_LEFT);
			map_w->callback(callback_resize_map);
			map_h = new Fl_Hor_Value_Slider(480,default_palette_bar_offset_y+104,312,24,"Map height");
			map_h->minimum(1);
			map_h->maximum(0xFFFF);
			map_h->step(1);
			map_h->value(2);
			map_h->align(FL_ALIGN_LEFT);
			map_h->callback(callback_resize_map);
			map_x_scroll = new Fl_Scrollbar(default_map_off_x-32, default_map_off_y-42, 800-8-default_map_off_x, 24);
			map_x_scroll->value(0,0,0,0);
			map_x_scroll->type(FL_HORIZONTAL);
			map_x_scroll->tooltip("Use this scroll bar to move around the tile map if you are zoomed in and there is not enough room to display the entire tilemap at once. This scroll bar will move the map left and right.");
			map_x_scroll->callback(update_map_scroll_x);
			map_x_scroll->hide();
			
			map_y_scroll = new Fl_Scrollbar(default_map_off_x-32, default_map_off_y, 24, 600-8-default_map_off_y);
			map_y_scroll->value(0,0,0,0);
			//map_x_scroll->type(FL_HORIZONTAL);
			map_y_scroll->tooltip("Use this scroll bar to move around the tile map if you are zoomed in and there is not enough room to display the entire tilemap at once. This scroll bar will move the map up and down.");
			map_y_scroll->callback(update_map_scroll_y);
			map_y_scroll->hide();
			
			//now for the tile select slider
			tile_select_2 = new Fl_Hor_Value_Slider(480,default_palette_bar_offset_y+40,312,24,"Tile Select");
			tile_select_2->tooltip("This slider allows you to choice which tile you would like to place on the map remember you can both horizontally and vertically flip the tile once placed on the map and select which row the tile uses");
			tile_select_2->minimum(0);
			tile_select_2->maximum(0);
			tile_select_2->step(1);
			tile_select_2->value(0);
			tile_select_2->align(FL_ALIGN_LEFT);
			tile_select_2->callback(set_tile_currentTP);
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
			{ Fl_Check_Button* o = new Fl_Check_Button(tile_place_buttons_x_off,184,64,32,"Show only selected row");
				o->callback(toggleRowSolo);
				o->tooltip("When checked Tiles that do not use the selected row will not be drawn");
			}
			{ hflipCB = new Fl_Check_Button(tile_place_buttons_x_off,336,64,32,"hflip");
				hflipCB->callback(set_hflipCB);
				hflipCB->tooltip("This sets whether or not the tile is flipped horizontally");
			}
			{ vflipCB = new Fl_Check_Button(tile_place_buttons_x_off,368,64,32,"vflip");
				vflipCB->callback(set_vflipCB);
				vflipCB->tooltip("This sets whether or not the tile is flipped vertically");
			}
			{ prioCB = new Fl_Check_Button(tile_place_buttons_x_off,400,72,32,"priority");
				prioCB->callback(set_prioCB);
				prioCB->tooltip("If checked tile is high priority");
			}
			{ Fl_Check_Button* o = new Fl_Check_Button(tile_place_buttons_x_off,432,96,32,"Show grid?");
				o->callback(set_grid_placer);
				o->tooltip("This button Toggles whether or not a grid is visible over the tilemap this will allow you to easily see were each tile is");
			}
			place_tile_size = new Fl_Hor_Value_Slider(tile_place_buttons_x_off,496,160,24,"Tile Zoom Factor:");
			place_tile_size->minimum(1);
			place_tile_size->maximum(16);
			place_tile_size->step(1);
			place_tile_size->value(12);
			place_tile_size->align(FL_ALIGN_TOP);
			place_tile_size->callback(update_map_size);
			place_tile_size->tooltip("By changing this slider you are changing the magnification of the tile for example if this slider was set to 10 that would mean that the tile is magnified by a factor of 10");
			TabsMain[2]->end();
		}
		{TabsMain[3] = new Fl_Group(rx,ry,rw,rh,"Chuck editor");
			TabsMain[3]->end();
		}
		{TabsMain[4] = new Fl_Group(rx,ry,rw,rh,"Settings/projects");
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
			
			sharePrj[0]=new Fl_Check_Button(8,88,112,16,"Share palette");
			sharePrj[0]->callback(shareProjectCB,(void*)pjHavePal);
			sharePrj[1]=new Fl_Check_Button(120,88,96,16,"Share Tiles");
			sharePrj[1]->callback(shareProjectCB,(void*)pjHaveTiles);
			sharePrj[2]=new Fl_Check_Button(216,88,120,16,"Share TileMap");
			sharePrj[2]->callback(shareProjectCB,(void*)pjHaveMap);
			
			havePrj[0]=new Fl_Check_Button(344,88,112,16,"Have palette");
			havePrj[0]->callback(haveCB,(void*)pjHavePal);
			havePrj[1]=new Fl_Check_Button(456,88,96,16,"Have Tiles");
			havePrj[1]->callback(haveCB,(void*)pjHaveTiles);
			havePrj[2]=new Fl_Check_Button(552,88,120,16,"Have TileMap");
			havePrj[2]->callback(haveCB,(void*)pjHaveMap);
			
			shareWith[0]=new Fl_Hor_Value_Slider(8,118,128,24,"Share Palette with:");
			shareWith[1]=new Fl_Hor_Value_Slider(136,118,128,24,"Share tiles with:");
			shareWith[2]=new Fl_Hor_Value_Slider(264,118,128,24,"Share TileMap with:");
			shareWith[0]->callback(switchShareCB,(void*)pjHavePal);
			shareWith[1]->callback(switchShareCB,(void*)pjHaveTiles);
			shareWith[2]->callback(switchShareCB,(void*)pjHaveMap);
			for(int x=0;x<3;++x){
				havePrj[x]->value(1);
				shareWith[x]->minimum(0);
				shareWith[x]->maximum(0);
				shareWith[x]->step(1);
				shareWith[x]->value(0);
				shareWith[x]->align(FL_ALIGN_TOP);
			}
			
			TxtBufProject = new Fl_Text_Buffer;
			TxtEditProject = new Fl_Text_Editor(8, 160, 640, 370,"Description/Notes");
			TxtEditProject->buffer(TxtBufProject);
			TxtEditProject->textfont(FL_TIMES);
			TxtBufProject->text(currentProject->Name.c_str());
			std::fill(tabsHidden,&tabsHidden[4],false);
			TabsMain[4]->end();
		}
	}
}
