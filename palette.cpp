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
typedef std::pair<double,int> HLSpair;
bool comparatorHLS(const HLSpair& l,const HLSpair& r)
   { return l.first < r.first; }
void sortBy(unsigned type,bool perRow){
	pushPaletteAll();
	HLSpair* MapHLS=new HLSpair[currentProject->colorCnt+currentProject->colorCntalt];
	for(unsigned x=0;x<currentProject->colorCnt+currentProject->colorCntalt*3;x+=3){
		double h,l,s;
		rgbToHls(currentProject->rgbPal[x],currentProject->rgbPal[x+1],currentProject->rgbPal[x+2],&h,&l,&s);
		MapHLS[x/3].first=pickIt(h,l,s,type);
		MapHLS[x/3].second=x/3;
	}
	if(perRow){
		for(unsigned i=0;i<4;++i)
			std::sort(MapHLS+(palEdit.perRow*i),MapHLS+(palEdit.perRow*(i+1)),comparatorHLS);
	}else
		std::sort(MapHLS,MapHLS+(currentProject->colorCnt+currentProject->colorCntalt),comparatorHLS);
	unsigned eSize;
	switch(currentProject->gameSystem){
		case sega_genesis:
			eSize=2;
			break;
		case NES:
			eSize=1;
			break;
	}
	uint8_t* newPal=(uint8_t*)alloca(currentProject->colorCnt+currentProject->colorCntalt*eSize);
	uint8_t* newPalRgb=(uint8_t*)alloca(currentProject->colorCnt+currentProject->colorCntalt*eSize*3);
	uint8_t* newPalType=(uint8_t*)alloca(currentProject->colorCnt+currentProject->colorCntalt);
	for(unsigned x=0;x<currentProject->colorCnt+currentProject->colorCntalt;++x){
		memcpy(newPal+(x*eSize),currentProject->palDat+(MapHLS[x].second*eSize),eSize);
		memcpy(newPalRgb+(x*3),currentProject->rgbPal+(MapHLS[x].second*3),3);
		newPalType[x]=currentProject->palType[MapHLS[x].second];
	}
	memcpy(currentProject->palDat,newPal,currentProject->colorCnt+currentProject->colorCntalt*eSize);
	memcpy(currentProject->rgbPal,newPalRgb,currentProject->colorCnt+currentProject->colorCntalt*3);
	memcpy(currentProject->palType,newPalType,currentProject->colorCnt+currentProject->colorCntalt);
	delete[] MapHLS;
}
void swapEntry(uint8_t one,uint8_t two){
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
			{uint8_t palOld=currentProject->palDat[two+two];
			memcpy(currentProject->palDat+two,currentProject->palDat+one,1);
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
	for (unsigned pal=0; pal < 128;pal+=2){
		//to convert to rgb first get value of color then multiply it by 16 to get rgb
		//first get blue value
		//the rgb array is in rgb format and the genesis palette is bgr format
		uint8_t rgb_array = pal+(pal/2);//multiply pal by 1.5
		uint8_t temp_var = currentProject->palDat[pal];
		temp_var>>=1;
		currentProject->rgbPal[rgb_array+2]=palTab[temp_var+type];
		//seperating the gr values will require some bitwise operations
		//to get g shift to the right by 4
		temp_var = currentProject->palDat[pal+1];
		temp_var>>=5;
		currentProject->rgbPal[rgb_array+1]=palTab[temp_var+type];
		//to get r value apply the and opperation by 0xF or 15
		temp_var = currentProject->palDat[pal+1];
		temp_var&=0xF;
		temp_var>>=1;
		currentProject->rgbPal[rgb_array]=palTab[temp_var+type];
	}
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
