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
//Color conversion functions should go here
#pragma once
#include <stdint.h>
#include "nespal.h"
extern uint8_t nespaltab[];
extern uint8_t nespaltab_alt[];
uint8_t nearest_color_index(uint8_t val,unsigned startindex);
uint8_t nearest_color_index(uint8_t val);
void rgbToHsl255(unsigned r,unsigned g,unsigned b,double * hh,double * ll,double * ss);
void rgbToHsl(double r,double g,double b,double * hh,double * ll,double * ss);
uint32_t count_colors(uint8_t * image_ptr,uint32_t w,uint32_t h,uint8_t *colors_found,bool useAlpha=false);
void update_emphesis(Fl_Widget*,void*);
static inline uint32_t toNesRgb(uint8_t ri,uint8_t gi,uint8_t bi){
	return nesPalToRgb(currentProject->pal->to_nes_color_rgb(ri,gi,bi));
}
void updateNesTab(unsigned emps,bool alt);
static inline double pickIt(double h,double s,double l,unsigned type){
	switch(type){
		case 0:
			return h;
		break;
		case 1:
			return s;
		break;
		case 2:
			return l;
		break;
	}
}
