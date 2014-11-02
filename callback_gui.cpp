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
#include "lua.h"
#include "CIE.h"
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
	unsigned bdold=bd;
	unsigned perRow,rows;
	switch(sel){
		case sega_genesis:
			perRow=16;
			rows=4;
		break;
		case NES:
			perRow=4;
			rows=4;
		break;
		default:
			show_default_error
			return;
	}
	perRow=perRow*rows/currentProject->rowCntPal;//Handle unequal row amounts
	memset(currentProject->palType,0,perRow*rows);
	uint8_t*tmpPalRGB=(uint8_t*)alloca(perRow*rows*3);
	if(perRow>=palEdit.perRow){
		for(unsigned i=0,j=0;i<currentProject->colorCnt*3;i+=palEdit.perRow*3,j+=perRow*3){
			memcpy(tmpPalRGB+j,currentProject->rgbPal+i,palEdit.perRow*3);
			memset(tmpPalRGB+j+((palEdit.perRow)*3),0,(perRow-palEdit.perRow)*3);
		}
	}else{
		uint8_t*nPtr=tmpPalRGB;
		uint8_t*rgbPtr=currentProject->rgbPal;
		for(unsigned k=0;k<rows;++k){
			//Preserve background color
			*nPtr++=rgbPtr[0];
			*nPtr++=rgbPtr[1];
			*nPtr++=rgbPtr[2];
			rgbPtr+=palEdit.perRow/perRow*3;
			for(unsigned j=(palEdit.perRow/perRow)*3;j<palEdit.perRow*3;j+=(palEdit.perRow/perRow)*3){
				unsigned type=0;
				double Lv,Cv,Hv;
				Rgb2Lch255(&Lv,&Cv,&Hv,rgbPtr[0],rgbPtr[1],rgbPtr[2]);
				rgbPtr+=3;
				for(unsigned i=1;i<palEdit.perRow/perRow;++i){
					double L,C,H;
					Rgb2Lch255(&L,&C,&H,rgbPtr[0],rgbPtr[1],rgbPtr[2]);
					if(type){
						if(C*L>Cv*Lv){
							Lv=L;
							Cv=C;
							Hv=H;
						}
					}else{
						if(C>Cv){
							Lv=L;
							Cv=C;
							Hv=H;
						}

					}
					rgbPtr+=3;
				}
				Lch2Rgb255(nPtr,nPtr+1,nPtr+2,Lv,Cv,Hv);
				nPtr+=3;
				type^=1;
			}
		}
	}
	tiles tilesOld=tiles(*currentProject->tileC);
	uint32_t gold=currentProject->gameSystem;
	uint32_t sold=currentProject->subSystem;
	switch(sel){
		case sega_genesis:
			bd=4;
			currentProject->gameSystem=sega_genesis;
			currentProject->subSystem=0;
			setBitdepthcurSys(bd);
			if(containsDataCurProj(pjHavePal)){
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
		break;
		case NES:
			bd=2;
			currentProject->gameSystem=NES;
			currentProject->subSystem=0;
			setBitdepthcurSys(bd);
			updateNesTab(0,false);
			updateNesTab(0,true);
			if(containsDataCurProj(pjHavePal)){
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
		break;
	}
	uint8_t*nPtr=tmpPalRGB;
	for(unsigned i=0;i<currentProject->colorCnt;++i,nPtr+=3)
		rgbToEntry(nPtr[0],nPtr[1],nPtr[2],i);
	if(currentProject->haveAltspritePal){
		memcpy(currentProject->rgbPal+(currentProject->colorCnt*3),currentProject->rgbPal,std::min(currentProject->colorCnt,currentProject->colorCntalt)*3);
		unsigned esize;
		switch(currentProject->gameSystem){
			case sega_genesis:
				fl_alert("Error the sega genesis does not support an alternative sprite palette");
				esize=0;
			break;
			case NES:
				esize=1;
			break;
		}
		memcpy(currentProject->palDat+(currentProject->colorCnt*esize),currentProject->palDat,std::min(currentProject->colorCnt,currentProject->colorCntalt)*esize);
	}
	window->redraw();
	uint32_t gnew=currentProject->gameSystem;
	uint32_t snew=currentProject->subSystem;
	for(unsigned i=0;i<tilesOld.amt;++i){
		for(unsigned y=0;y<std::min(currentProject->tileC->sizeh,tilesOld.sizeh);++y){
			for(unsigned x=0;x<std::min(currentProject->tileC->sizew,tilesOld.sizew);++x){
				currentProject->gameSystem=gold;
				currentProject->subSystem=sold;
				uint32_t px=tilesOld.getPixel(i,x,y);
				if(bdold>bd)
					px>>=bdold-bd;
				currentProject->gameSystem=gnew;
				currentProject->subSystem=snew;
				currentProject->tileC->setPixel(i,x,y,px);
			}
		}
	}
}
void trueColTileToggle(Fl_Widget*,void*){
	showTrueColor^=1;
	window->damage(FL_DAMAGE_USER1);
}
void toggleRowSolo(Fl_Widget*,void*){
	rowSolo^=true;
	window->redraw();
}
