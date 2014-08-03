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
#include <stdint.h>
class sprite{
	public:
		uint32_t w,h;//In tiles
		uint32_t starttile;
		uint32_t palrow;
		bool hflip,vflip;
		sprite();
		sprite(uint32_t wi,uint32_t hi,uint32_t palrowset,uint32_t settile,bool hf,bool vf);
		void draw(unsigned x,unsigned y,unsigned zoom);
};
