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
   Copyright Sega16 (or whatever you wish to call me) (2012-2017)
*/
#pragma once
#include "project.h"
#include <stdint.h>
struct sprite {
	Project*prj;
	uint32_t w, h; //In tiles
	uint32_t starttile, palrow;
	uint32_t loadat;//Where the sprite will be loaded in game (useful for games that overwrite tiles in VRAM for animation purposes such as the Sonic series of games and many more.)
	int32_t offx, offy;
	bool hflip, vflip;
	bool prio;
	sprite(Project*prj);
	sprite(uint32_t wi, uint32_t hi, uint32_t palrowset, uint32_t settile, bool hf, bool vf, bool pri, uint32_t la, int32_t ox, int32_t oy, Project*prj);
	void draw(unsigned x, unsigned y, unsigned zoom);
	void toImage(uint8_t*img);
};
