#pragma once
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
unsigned find_near_color_from_row_rgb(unsigned row,int r,int g,int b,bool alt);
unsigned find_near_color_from_row(unsigned row,int r,int g,int b,bool alt);
unsigned choiceTwoColor(unsigned index0,unsigned index1,int rgoal,int ggoal,int bgoal);
unsigned nearestOneChannel(int val,const uint8_t*pal,unsigned amt);
unsigned nearestColIndex(int red,int green,int blue,uint8_t*pal,unsigned amt,bool checkType=false,unsigned off=0);
