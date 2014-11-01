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
unsigned calMaxPerRow(unsigned row){
	row*=palEdit.perRow;
	unsigned max=palEdit.perRow;
	for(unsigned i=row;i<palEdit.perRow+row;++i){
		if(currentProject->palType[i]&&max)//Locked or reserved colors cannot be changed
			--max;
	}
	return max;
}
unsigned palTypeGen;
typedef std::pair<double,int> HSLpair;
bool comparatorHSL(const HSLpair& l,const HSLpair& r)
   { return l.first < r.first; }
void sortBy(unsigned type,bool perRow){
	pushPaletteAll();
	unsigned totalCol=currentProject->colorCnt+currentProject->colorCntalt;
	HSLpair* MapHSL=new HSLpair[totalCol];
	for(unsigned x=0;x<totalCol*3;x+=3){
		double h,l,s;
		rgbToHsl(currentProject->rgbPal[x],currentProject->rgbPal[x+1],currentProject->rgbPal[x+2],&h,&s,&l);
		MapHSL[x/3].first=pickIt(h,s,l,type);
		MapHSL[x/3].second=x/3;
	}
	if(perRow){
		for(unsigned i=0;i<currentProject->rowCntPal+currentProject->rowCntPalalt;++i)
			std::sort(MapHSL+(palEdit.perRow*i),MapHSL+(palEdit.perRow*(i+1)),comparatorHSL);
	}else
		std::sort(MapHSL,MapHSL+(totalCol),comparatorHSL);
	unsigned eSize;
	switch(currentProject->gameSystem){
		case sega_genesis:
			eSize=2;
		break;
		case NES:
			eSize=1;
		break;
	}
	uint8_t* newPal=(uint8_t*)alloca((totalCol)*eSize);
	uint8_t* newPalRgb=(uint8_t*)alloca(totalCol*eSize*3);
	uint8_t* newPalType=(uint8_t*)alloca(totalCol);
	for(unsigned x=0;x<totalCol;++x){
		memcpy(newPal+(x*eSize),currentProject->palDat+(MapHSL[x].second*eSize),eSize);
		memcpy(newPalRgb+(x*3),currentProject->rgbPal+(MapHSL[x].second*3),3);
		newPalType[x]=currentProject->palType[MapHSL[x].second];
	}
	memcpy(currentProject->palDat,newPal,totalCol*eSize);
	memcpy(currentProject->rgbPal,newPalRgb,totalCol*3);
	memcpy(currentProject->palType,newPalType,totalCol);
	delete[] MapHSL;
}
void swapEntry(unsigned one,unsigned two){
	if(unlikely(one==two))
		return;
	switch(currentProject->gameSystem){
		case sega_genesis:
			{uint8_t palOld[2];
			memcpy(palOld,currentProject->palDat+two+two,2);
			memcpy(currentProject->palDat+two+two,currentProject->palDat+one+one,2);
			memcpy(currentProject->palDat+one+one,palOld,2);}
		break;
		case NES:
			{uint8_t palOld=currentProject->palDat[two];
			currentProject->palDat[two]=currentProject->palDat[one];
			currentProject->palDat[one]=palOld;}
		break;
		default:
			show_default_error
	}
	uint8_t rgb[3];
	memcpy(rgb,currentProject->rgbPal+(two*3),3);
	memcpy(currentProject->rgbPal+(two*3),currentProject->rgbPal+(one*3),3);
	memcpy(currentProject->rgbPal+(one*3),rgb,3);
}
const uint8_t palTab[]=   {0,49,87,119,146,174,206,255,0,27,49,71,87,103,119,130,130,146,157,174,190,206,228,255};//from http://gendev.spritesmind.net/forum/viewtopic.php?t=1389
const uint8_t palTabEmu[]={0,36,72,108,144,180,216,252,0,18,36,54,72, 90,108,126,126,144,162,180,198,216,234,252};
void set_palette_type_force(unsigned type){
	palTypeGen=type;
	//now reconvert all the colors
	for(unsigned pal=0; pal<64;++pal)
		updateRGBindex(pal);
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
