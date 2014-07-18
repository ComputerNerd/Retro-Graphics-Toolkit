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
#include "global.h"
#include "color_compare.h"
#include "color_convert.h"
void update_box_size(Fl_Widget*, void*){
	window->redraw();
}
void showAbout(Fl_Widget*,void*){
	fl_alert("Retro Graphics Toolkit is written by sega16/nintendo8/sonic master or whatever username you know me as\nThis program was built on %s %s\n",__DATE__,__TIME__);
}
void set_game_system(Fl_Widget*,void* selection){
	uint32_t sel8=(uintptr_t)selection;
	if (unlikely(sel8 == currentProject->gameSystem)){
		fl_alert("You are already in that mode");
		return;
	}
	switch(sel8){
		case sega_genesis:
			//fl_alert("Sega genesis Mode");
			currentProject->gameSystem=sega_genesis;
			currentProject->tileC->tileSize=32;
			//create_shadow_highlight_buttons();
			shadow_highlight_switch->show();
			{//for varibles to be declared inside of switch statment I must put brackes around so the compiler knows when to free them
				uint8_t pal_temp[128];
				uint8_t c;
				for (c=0;c<128;c+=2){
					uint16_t temp=to_sega_genesis_color(c/2);
					pal_temp[c]=temp>>8;
					pal_temp[c+1]=temp&255;
				}
				memcpy(currentProject->palDat,pal_temp,128);
			}
			palEdit.changeSystem();
			tileEdit_pal.changeSystem();
			tileMap_pal.changeSystem();
			currentProject->tileC->tileDat = (uint8_t *)realloc(currentProject->tileC->tileDat,(currentProject->tileC->tiles_amount+1)*32);
			window->map_w->step(1);
			window->map_h->step(1);
		break;
		case NES:
			currentProject->gameSystem=NES;
			currentProject->tileC->tileSize=16;
			shadow_highlight_switch->hide();
			updateNesTab(0);
			for (uint8_t c=0;c<16;c++)
				currentProject->palDat[c]=to_nes_color(c);
			palEdit.changeSystem();
			tileEdit_pal.changeSystem();
			tileMap_pal.changeSystem();
			update_emphesis(0,0);
			currentProject->tileC->tileDat = (uint8_t *)realloc(currentProject->tileC->tileDat,(currentProject->tileC->tiles_amount+1)*16);
			//on the NES tilemaps need to be a multiple of 2
			if(currentProject->subSystem==NES2x2){
				if(((currentProject->tileMapC->mapSizeW)&1) && ((currentProject->tileMapC->mapSizeHA)&1))
					currentProject->tileMapC->resize_tile_map(currentProject->tileMapC->mapSizeW+1,currentProject->tileMapC->mapSizeHA+1);
				if((currentProject->tileMapC->mapSizeW)&1)
					currentProject->tileMapC->resize_tile_map(currentProject->tileMapC->mapSizeW+1,currentProject->tileMapC->mapSizeHA);
				if((currentProject->tileMapC->mapSizeHA)&1)
					currentProject->tileMapC->resize_tile_map(currentProject->tileMapC->mapSizeW,currentProject->tileMapC->mapSizeHA+1);
			}
			window->map_w->value(currentProject->tileMapC->mapSizeW);
			window->map_h->value(currentProject->tileMapC->mapSizeHA);
			window->map_w->step(2);
			window->map_h->step(2);
		break;
		default:
			show_default_error
			return;
		break;
	}
	if(currentProject->gameSystem==NES){
		window->subSysC->show();
		window->subSysC->value(currentProject->subSystem);
	}else
		window->subSysC->hide();
	window->redraw();
}
void trueColTileToggle(Fl_Widget*,void*){
	showTrueColor^=1;
	window->damage(FL_DAMAGE_USER1);
}
void toggleRowSolo(Fl_Widget*,void*){
	rowSolo^=true;
	window->redraw();
}
