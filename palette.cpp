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
#include "errorMsg.h"
#include "color_convert.h"
#include "undo.h"
unsigned palTypeGen;
typedef std::pair<double,int> HSLpair;
static bool comparatorHSL(const HSLpair& l,const HSLpair& r){
	return l.first < r.first;
}
void sortBy(unsigned type,bool perRow){
	pushPaletteAll();
	unsigned totalCol=currentProject->pal->colorCnt+currentProject->pal->colorCntalt;
	HSLpair* MapHSL=new HSLpair[totalCol];
	for(unsigned x=0;x<totalCol*3;x+=3){
		double h,l,s;
		rgbToHsl255(currentProject->pal->rgbPal[x],currentProject->pal->rgbPal[x+1],currentProject->pal->rgbPal[x+2],&h,&s,&l);
		MapHSL[x/3].first=pickIt(h,s,l,type);
		MapHSL[x/3].second=x/3;
	}
	if(perRow){
		for(unsigned i=0;i<currentProject->pal->rowCntPal+currentProject->pal->rowCntPalalt;++i)
			std::sort(MapHSL+(currentProject->pal->perRow*i),MapHSL+(currentProject->pal->perRow*(i+1)),comparatorHSL);
	}else
		std::sort(MapHSL,MapHSL+(totalCol),comparatorHSL);
	uint8_t* newPal=(uint8_t*)alloca((totalCol)*currentProject->pal->esize);
	uint8_t* newPalRgb=(uint8_t*)alloca(totalCol*currentProject->pal->esize*3);
	uint8_t* newPalType=(uint8_t*)alloca(totalCol);
	for(unsigned x=0;x<totalCol;++x){
		memcpy(newPal+(x*currentProject->pal->esize),currentProject->pal->palDat+(MapHSL[x].second*currentProject->pal->esize),currentProject->pal->esize);
		memcpy(newPalRgb+(x*3),currentProject->pal->rgbPal+(MapHSL[x].second*3),3);
		newPalType[x]=currentProject->pal->palType[MapHSL[x].second];
	}
	memcpy(currentProject->pal->palDat,newPal,totalCol*currentProject->pal->esize);
	memcpy(currentProject->pal->rgbPal,newPalRgb,totalCol*3);
	memcpy(currentProject->pal->palType,newPalType,totalCol);
	delete[] MapHSL;
}
const uint8_t palTabGameGear[]={0,17,34,51,68,85,102,119,136,153,170,187,204,221,236,255};
const uint8_t palTabMasterSystem[]={0,85,170,255};//From http://segaretro.org/Palette
const uint8_t palTab[]=   {0,49,87,119,146,174,206,255,0,27,49,71,87,103,119,130,130,146,157,174,190,206,228,255};//from http://gendev.spritesmind.net/forum/viewtopic.php?t=1389
const uint8_t palTabEmu[]={0,36,72,108,144,180,216,252,0,18,36,54,72, 90,108,126,126,144,162,180,198,216,234,252};
void set_palette_type_force(unsigned type){
	palTypeGen=type;
	//now reconvert all the colors
	for(unsigned pal=0; pal<64;++pal)
		currentProject->pal->updateRGBindex(pal);
}
void set_palette_type(void){
	if(currentProject->subSystem&sgSon){
		if(currentProject->subSystem&sgHon)
			set_palette_type_force(16);
		else
			set_palette_type_force(8);
	}else
		set_palette_type_force(0);
}
