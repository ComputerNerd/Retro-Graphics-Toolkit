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
#include "includes.h"
#include "class_global.h"
#include "errorMsg.h"
#include "dither.h"
#include "class_tiles.h"
#include "color_convert.h"
#include "color_compare.h"
#include "nearestColor.h"
#include "classpalettebar.h"
#include "global.h"
//bools used to toggle stuff
bool show_grid;
bool G_hflip[2];
bool G_vflip[2];
bool G_highlow_p[2];
bool show_grid_placer;
//moveable offsets
unsigned tile_zoom_edit;
uint8_t truecolor_temp[4];/*!< This stores the rgba data selected with the truecolor sliders*/
std::string the_file;//this is for temporary use only
unsigned mode_editor;//this is used to determine which thing to draw
bool showTrueColor;
bool rowSolo;
bool tileEditModePlace_G;
uint32_t selTileE_G[2];
void tileToTrueCol(uint8_t * input,uint8_t * output,uint8_t row,bool useAlpha,bool alphaZero){
	switch (currentProject->gameSystem){
		case segaGenesis:
			row*=48;
			for (uint8_t y=0;y<8;++y){
				for (uint8_t x=0;x<4;++x){
					//even,odd
					uint8_t temp=*input++;
					uint8_t temp_1,temp_2;
					temp_1=temp>>4;//first pixel
					temp_2=temp&15;//second pixel
					*output++=currentProject->pal->rgbPal[row+(temp_1*3)];
					*output++=currentProject->pal->rgbPal[row+(temp_1*3)+1];
					*output++=currentProject->pal->rgbPal[row+(temp_1*3)+2];
					if(useAlpha){
						if(alphaZero){
							if(temp_1)
								*output++=255;
							else
								*output++=0;
						}else
							*output++=255;
					}
					*output++=currentProject->pal->rgbPal[row+(temp_2*3)];
					*output++=currentProject->pal->rgbPal[row+(temp_2*3)+1];
					*output++=currentProject->pal->rgbPal[row+(temp_2*3)+2];
					if(useAlpha){
						if(alphaZero){
							if(temp_2)
								*output++=255;
							else
								*output++=0;
						}else
							*output++=255;
					}
				}
			}
		break;
		case NES:
			row*=12;
			for (uint8_t y=0;y<8;++y){
				for (int8_t x=7;x>=0;--x){
					uint8_t temp;
					temp=(input[y]>>x)&1;
					temp|=((input[y+8]>>x)&1)<<1;
					*output++=currentProject->pal->rgbPal[row+(temp*3)];
					*output++=currentProject->pal->rgbPal[row+(temp*3)+1];
					*output++=currentProject->pal->rgbPal[row+(temp*3)+2];
					if(useAlpha){
						if(alphaZero){
							if(temp)
								*output++=255;
							else
								*output++=0;
						}else
							*output++=255;
					}
				}
			}
		break;
	}
}
bool verify_str_number_only(char * str){
/*!
Fltk provides an input text box that makes it easy for the user to type text however as a side effect they can accidentally enter non number characters that may be handled weird by atoi()
this function address that issue by error checking the string and it also gives the user feedback so they are aware that the input box takes only numbers
this function returns true when the string contains only numbers 0-9 and false when there is other stuff
it will also allow the use of the - symbol as negative
*/
	while(*str++){
		if (*str != 0 && *str != '-'){
			if (*str < '0'){
				fl_alert("Please enter only numbers in decimal format\nCharacter entered %c",*str);
				return false;
			}
			if (*str > '9'){
				fl_alert("Please enter only numbers in decimal format\nCharacter entered %c",*str);
				return false;
			}
		}
	}
	return true;
}
static inline uint32_t sq(uint32_t x){
	return x*x;
}
uint8_t find_near_color_from_row_rgb(unsigned row,uint8_t r,uint8_t g,uint8_t b,bool alt){
	row*=currentProject->pal->perRow;
	uint8_t*rgbPtr=currentProject->pal->rgbPal+(row*3);
	if((currentProject->gameSystem==NES)&&alt)
		rgbPtr+=16*3;
	return (nearestColIndex(r,g,b,rgbPtr,currentProject->pal->perRow,true,row)+row)*3;//Yes this function does return three times the value TODO refractor
}
uint8_t find_near_color_from_row(unsigned row,uint8_t r,uint8_t g,uint8_t b,bool alt){
	return (find_near_color_from_row_rgb(row,r,g,b,alt)/3)-(row*currentProject->pal->perRow);
}
uint32_t cal_offset_truecolor(unsigned x,unsigned y,unsigned rgb,uint32_t tile){
	/*!<
	cal_offset_truecolor is made to help when accessing a true color tile array
	an example of it would be
	red_temp=truecolor_data[cal_offset_truecolor(0,0,0,0)]//get the red pixel at pixel 0,0 at tile 0
	green_temp=truecolor_data[cal_offset_truecolor(0,0,1,0)]//get the green pixel at pixel 0,0 at tile 0
	blue_temp=truecolor_data[cal_offset_truecolor(0,0,2,0)]//get the blue pixel at pixel 0,0 at tile 0
	*/
	return (x*4)+(y*32)+rgb+(tile*256);
}
