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
	uint8_t rows;//tells how many rows there are in the palette
	uint16_t offx,offy;//the offset in which the palette selection boxes will appear
	uint16_t offxx,offyy;//the offset in which the palette selection boxes will appear
public:
	Fl_Slider* pal_r;
	Fl_Slider* pal_g;
	Fl_Slider* pal_b;
	uint8_t box_sel;/*!< tells what palette entry is selected*/
	uint8_t theRow;/*!< tells what row in the palette is selected*/
	uint8_t perRow;
	uint8_t getEntry(void);
	void check_box(int16_t,int16_t);
	void draw_boxes();
	void more_init(uint8_t x=1,uint16_t offsetx=16,uint16_t offsety=56);//this one should be called in a function that creates the gui elements
	void changeRow(uint8_t);
	void changeSystem();
	void updateSlider();
	void updateSize(void);
}
extern palEdit,tileEdit_pal,tileMap_pal;
