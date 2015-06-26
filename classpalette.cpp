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
   Copyright Sega16 (or whatever you wish to call me) (2012-2015)
*/
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "macros.h"
#include "palette.h"
#include "project.h"
#include "classpalette.h"
#include "system.h"
#include "color_convert.h"
#include "nearestColor.h"
static const uint8_t TMS9918Palette[]={
  0,   0,   0,
  0,   0,   0,
 33, 200,  66,
 94, 220, 120,
 84,  85, 237,
125, 118, 252,
212,  82,  77,
 66, 235, 245,
252,  85,  84,
255, 121, 120,
212, 193,  84,
230, 206, 128,
 33, 176,  59,
201,  91, 186,
204, 204, 204,
255, 255, 255};
palette::palette(void){
	rgbPal=0;
	setVars(currentProject->gameSystem);
	memset(rgbPal,0,(colorCnt+colorCntalt)*(4+esize));
}
palette::~palette(void){
	free(rgbPal);
}
palette::palette(const palette& other){
	rgbPal=0;
	setVars(currentProject->gameSystem);
	memcpy(rgbPal,other.rgbPal,std::min((colorCnt+colorCntalt)*(4+esize),(other.colorCnt+other.colorCntalt)*(4+other.esize)));
}
void palette::setVars(enum gameSystemEnum gameSystem){
	switch(gameSystem){
		case segaGenesis:
			rowCntPal=4;
			colorCnt=64;
			colorCntalt=0;
			rowCntPalalt=0;
			perRow=16;
			perRowalt=0;
			haveAlt=false;
			esize=2;
		break;
		case NES:
			colorCnt=colorCntalt=16;
			rowCntPal=rowCntPalalt=4;
			perRowalt=perRow=4;
			haveAlt=true;
			esize=1;
		break;
		case masterSystem:
		case gameGear:
			rowCntPal=2;
			colorCnt=32;
			colorCntalt=0;
			rowCntPalalt=0;
			perRow=16;
			perRowalt=0;
			haveAlt=false;
			esize=gameSystem==gameGear?2:1;
		break;
		case TMS9918:
			rowCntPal=1;
			colorCnt=16;
			colorCntalt=0;
			rowCntPalalt=0;
			perRow=16;
			perRowalt=0;
			haveAlt=false;
			esize=0;
		break;
		default:
			showGameSysError(gameSystem)
	}
	rgbPal=(uint8_t*)realloc(rgbPal,(colorCnt+colorCntalt)*(4+esize));//Yes this is correct when rgbPal is NULL realloc will behave the same as malloc
	if(esize)
		palDat=rgbPal+((colorCnt+colorCntalt)*3);
	else
		palDat=0;
	palType=rgbPal+((colorCnt+colorCntalt)*(3+esize));
	if(gameSystem==TMS9918)
		memcpy(rgbPal,TMS9918Palette,sizeof(TMS9918Palette));
}
void palette::read(FILE*fp,bool supportsAlt){
	if(supportsAlt){
		fread(palDat,colorCnt+colorCntalt,esize,fp);
		fread(palType,colorCnt+colorCntalt,1,fp);
	}else{
		fread(palDat,colorCnt,esize,fp);
		fread(palType,colorCnt,1,fp);
		if(haveAlt){
			if(palDat)
				memset(palDat+(colorCnt*esize),0,colorCntalt*esize);
			memset(palType+colorCnt,0,colorCntalt);
		}
	}
}
void palette::write(FILE*fp){
	fwrite(palDat,colorCnt+colorCntalt,esize,fp);
	fwrite(palType,colorCnt+colorCntalt,1,fp);
}
void palette::updateRGBindex(unsigned index){
	switch(currentProject->gameSystem){
		case segaGenesis:
			{uint16_t*ptr=(uint16_t*)palDat+index;
			rgbPal[index*3+2]=palTab[((*ptr>>1)&7)+palTypeGen];//Blue note that bit shifting is different due to little endian
			rgbPal[index*3+1]=palTab[((*ptr>>13)&7)+palTypeGen];//Green
			rgbPal[index*3]=palTab[((*ptr>>9)&7)+palTypeGen];//Red
			}
		break;
		case NES:
			{uint32_t rgb_out=nesPalToRgb(palDat[index]);
			rgbPal[index*3+2]=rgb_out&255;//blue
			rgbPal[index*3+1]=(rgb_out>>8)&255;//green
			rgbPal[index*3]=(rgb_out>>16)&255;//red
			}
		break;
		case masterSystem:
			rgbPal[index*3]=palTabMasterSystem[palDat[index]&3];
			rgbPal[index*3+1]=palTabMasterSystem[(palDat[index]>>2)&3];
			rgbPal[index*3+2]=palTabMasterSystem[(palDat[index]>>4)&3];
		break;
		case gameGear:
			{uint16_t*ptr=(uint16_t*)palDat+index;
			rgbPal[index*3]=palTabGameGear[*ptr&15];
			rgbPal[index*3+1]=palTabGameGear[(*ptr>>4)&15];
			rgbPal[index*3+2]=palTabGameGear[(*ptr>>8)&15];}
		break;
		case TMS9918:
			//Do nothing
		break;
		default:
			show_default_error
	}
}
void palette::clear(void){
	memset(rgbPal,0,(colorCnt+colorCntalt)*(3+esize));
}
void palette::rgbToEntry(unsigned r,unsigned g,unsigned b,unsigned ent){
	unsigned maxent=colorCnt+colorCntalt;
	if(ent>maxent){
		fl_alert("Attempted access for color %d but there is only %d colors",ent,maxent);
		return;
	}
	switch(currentProject->gameSystem){
		case segaGenesis:
			{uint16_t temp=to_sega_genesis_colorRGB(r,g,b,ent);
			ent*=2;
			palDat[ent]=temp>>8;
			palDat[ent+1]=temp&255;}
		break;
		case NES:
			palDat[ent]=to_nes_color_rgb(r,g,b);
			updateRGBindex(ent);
		break;
		case masterSystem:
			palDat[ent]=nearestOneChannel(r,palTabMasterSystem,4);
			palDat[ent]|=nearestOneChannel(g,palTabMasterSystem,4)<<2;
			palDat[ent]|=nearestOneChannel(b,palTabMasterSystem,4)<<4;
		break;
		case gameGear:
			{uint16_t*palPtr=(uint16_t*)palDat+ent;
			*palPtr=nearestOneChannel(r,palTabGameGear,16);
			*palPtr|=nearestOneChannel(g,palTabGameGear,16)<<4;
			*palPtr|=nearestOneChannel(b,palTabGameGear,16)<<8;}
		break;
		case TMS9918:
			//Do nothing
		break;
		default:
			show_default_error
	}
}
uint8_t palette::to_nes_color_rgb(uint8_t red,uint8_t green,uint8_t blue){
	//this function does not set any values to global palette it is done in other functions
	return nearestColIndex(red,green,blue,nespaltab,64);
}
uint8_t palette::to_nes_color(unsigned pal_index){
	//this function does not set any values to global palette it is done in other functions
	pal_index*=3;
	return to_nes_color_rgb(rgbPal[pal_index],rgbPal[pal_index+1],rgbPal[pal_index+2]);
}
uint8_t palette::toNesChan(uint8_t ri,uint8_t gi,uint8_t bi,uint8_t chan){
	uint32_t rgb_out=toNesRgb(ri,gi,bi);
	uint8_t b=rgb_out&255;
	uint8_t g=(rgb_out>>8)&255;
	uint8_t r=(rgb_out>>16)&255;
	switch (chan){
		case 0:
			return r;
		break;
		case 1:
			return g;
		break;
		case 2:
			return b;
		break;
	}
	return 0;
}
uint16_t palette::to_sega_genesis_colorRGB(uint8_t r,uint8_t g,uint8_t b,uint16_t pal_index){
	//note this function only set the new rgb colors not the outputted sega genesis palette format
	pal_index*=3;
	r=nearest_color_index(r,0);
	g=nearest_color_index(g,0);
	b=nearest_color_index(b,0);
	rgbPal[pal_index]=palTab[r];
	rgbPal[pal_index+1]=palTab[g];
	rgbPal[pal_index+2]=palTab[b];
	//bgr format
	return (r<<1)|(g<<5)|(b<<9);
}
uint16_t palette::to_sega_genesis_color(uint16_t pal_index){
	//note this function only set the new rgb colors not the outputted sega genesis palette format
	pal_index*=3;
	uint8_t r,g,b;
	r=nearest_color_index(rgbPal[pal_index]);
	g=nearest_color_index(rgbPal[pal_index+1]);
	b=nearest_color_index(rgbPal[pal_index+2]);
	rgbPal[pal_index]=palTab[r];
	rgbPal[pal_index+1]=palTab[g];
	rgbPal[pal_index+2]=palTab[b];
	//bgr format
	return ((r-palTypeGen)<<1)|((g-palTypeGen)<<5)|((b-palTypeGen)<<9);
}
unsigned palette::calMaxPerRow(unsigned row){
	row*=perRow;
	unsigned max=perRow;
	for(unsigned i=row;i<perRow+row;++i){
		if(palType[i]&&max)//Locked or reserved colors cannot be changed
			--max;
	}
	return max;
}
void palette::swapEntry(unsigned one,unsigned two){
	if(unlikely(one==two))
		return;
	switch(esize){
		case 1:
			{uint8_t palOld=palDat[two];
			palDat[two]=palDat[one];
			palDat[one]=palOld;}
		break;
		case 2:
			{uint8_t palOld[2];
			memcpy(palOld,palDat+two+two,2);
			memcpy(palDat+two+two,palDat+one+one,2);
			memcpy(palDat+one+one,palOld,2);}
		break;
		default:
			show_default_error
	}
	uint8_t rgb[3];
	memcpy(rgb,rgbPal+(two*3),3);
	memcpy(rgbPal+(two*3),rgbPal+(one*3),3);
	memcpy(rgbPal+(one*3),rgb,3);
}
