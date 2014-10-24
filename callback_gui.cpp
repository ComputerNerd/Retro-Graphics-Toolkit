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
#include "color_compare.h"
#include "color_convert.h"
#include "system.h"
#include "callback_project.h"
#include "lua/lua.h"
static const char* GPLv3="This program is free software: you can redistribute it and/or modify\n"
	"it under the terms of the GNU General Public License as published by\n"
	"the Free Software Foundation, either version 3 of the License, or\n"
	"(at your option) any later version.\n\n"
	"This program is distributed in the hope that it will be useful,\n"
	"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
	"GNU General Public License for more details.\n\n"
	"You should have received a copy of the GNU General Public License\n"
	"along with this program.  If not, see <http://www.gnu.org/licenses/>.\n";
void setSubditherSetting(Fl_Widget*w,void*){
	Fl_Slider*s=(Fl_Slider*)w;
	currentProject->settings&=~(subsettingsDitherMask<<subsettingsDitherShift);
	currentProject->settings|=(((uint32_t)s->value()-1)&subsettingsDitherMask)<<subsettingsDitherShift;
}
void redrawOnlyCB(Fl_Widget*, void*){
	window->redraw();
}
void showAbout(Fl_Widget*,void*){
	fl_alert("Retro Graphics Toolkit is written by sega16/nintendo8/sonic master or whatever username you know me as\nhttps://github.com/ComputerNerd/Retro-Graphics-Toolkit\nThis program was built on %s %s\n\n%s\nUses lua version: " LUA_RELEASE "\n" LUA_COPYRIGHT "\n" LUA_AUTHORS,__DATE__,__TIME__,GPLv3);
}
Fl_Menu_Item subSysNES[]={
	{"1x1 tile palette",0,setNesTile,(void*)NES1x1},
	{"2x2 tile palette",0,setNesTile,(void*)NES2x2},
	{0}
};
Fl_Menu_Item subSysGenesis[]={
	{"Normal",0,setSegaPalType,(void*)0},
	{"Shadow",0,setSegaPalType,(void*)sgSon},
	{"Highlight",0,setSegaPalType,(void*)sgSHmask},
	{0}
};
void set_game_system(Fl_Widget*,void* selection){
	uint32_t sel=(uintptr_t)selection;
	if (unlikely(sel == currentProject->gameSystem)){
		fl_alert("You are already in that mode");
		return;
	}
	unsigned bd=getBitdepthcurSys();
	switch(sel){
		case sega_genesis:
			if(bd>4)
				bd=4;
			{uint32_t oldSys=currentProject->gameSystem;
			currentProject->gameSystem=sega_genesis;
			currentProject->subSystem=0;
			setBitdepthcurSys(bd);
			if(containsDataCurProj(pjHavePal)){
				if(oldSys==NES){
					uint8_t pal_temp[128];
					unsigned c;
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
				spritePal.changeSystem();
			}
			if(containsDataCurProj(pjHaveTiles)){
				currentProject->tileC->tileSize=32;
				currentProject->tileC->resizeAmt();
			}
			if(containsDataCurProj(pjHaveSprites)){
				window->spritesize[0]->maximum(4);
				window->spritesize[1]->maximum(4);
				currentProject->spritesC->enforceMax(4,4);
				window->updateSpriteSliders();
			}
			window->subSysC->copy(subSysGenesis);
			window->subSysC->value((currentProject->subSystem&sgSHmask)>>sgSHshift);
			}
		break;
		case NES:
			bd=getBitdepthcurSys();
			if(bd>2)
				bd=2;
			currentProject->gameSystem=NES;
			currentProject->subSystem=0;
			setBitdepthcurSys(bd);
			updateNesTab(0,false);
			updateNesTab(0,true);
			if(containsDataCurProj(pjHavePal)){
				for (unsigned c=0;c<32;++c)
					currentProject->palDat[c]=to_nes_color(c);
				palEdit.changeSystem();
				tileEdit_pal.changeSystem();
				tileMap_pal.changeSystem();
				spritePal.changeSystem();
				update_emphesis(0,0);
			}
			if(containsDataCurProj(pjHaveTiles)){
				currentProject->tileC->tileSize=16;
				currentProject->tileC->resizeAmt();
			}
			currentProject->subSystem|=NES2x2;
			if(containsDataCurProj(pjHaveMap)){
				//on the NES tilemaps need to be a multiple of 2
				if(((currentProject->tileMapC->mapSizeW)&1) && ((currentProject->tileMapC->mapSizeHA)&1))
					currentProject->tileMapC->resize_tile_map(currentProject->tileMapC->mapSizeW+1,currentProject->tileMapC->mapSizeHA+1);
				if((currentProject->tileMapC->mapSizeW)&1)
					currentProject->tileMapC->resize_tile_map(currentProject->tileMapC->mapSizeW+1,currentProject->tileMapC->mapSizeHA);
				if((currentProject->tileMapC->mapSizeHA)&1)
					currentProject->tileMapC->resize_tile_map(currentProject->tileMapC->mapSizeW,currentProject->tileMapC->mapSizeHA+1);
				window->updateMapWH();
			}
			if(containsDataCurProj(pjHaveSprites)){
				window->spritesize[0]->maximum(1);
				window->spritesize[1]->maximum(2);
				currentProject->spritesC->enforceMax(1,2);
				window->updateSpriteSliders();
			}
			window->subSysC->copy(subSysNES);
			window->subSysC->value(currentProject->subSystem&NES2x2);
		break;
		case frameBuffer_pal:
			{currentProject->gameSystem=frameBuffer_pal;
			currentProject->subSystem=0;
			if(bd>8)
				bd=8;
			setBitdepthcurSys(bd);
			}
		break;
		default:
			show_default_error
			return;
		break;
	}
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
