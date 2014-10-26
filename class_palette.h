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
#pragma once
#include "global.h"
class palette_bar{
	unsigned rows;//tells how many rows there are in the palette
	unsigned offx,offy;//the offset in which the palette selection boxes will appear
	unsigned offxx,offyy;//the offset in which the palette selection boxes will appear
	uint32_t sysCache;
	bool alt;
	void setSysColCnt(void);
public:
	Fl_Slider* pal_r;
	Fl_Slider* pal_g;
	Fl_Slider* pal_b;
	unsigned box_sel;/*!< tells what palette entry is selected*/
	unsigned theRow;/*!< tells what row in the palette is selected*/
	unsigned perRow;
	unsigned getEntry(void);
	void check_box(int,int);
	void draw_boxes(void);
	void more_init(uint8_t x=1,uint16_t offsetx=16,uint16_t offsety=56,bool altset=false,unsigned ln=256,bool tiny=false);//this one should be called in a function that creates the gui elements
	void changeRow(uint8_t);
	void changeSystem(void);
	void updateSlider(void);
	void updateSize(void);
}
extern palEdit,tileEdit_pal,tileMap_pal,spritePal;
