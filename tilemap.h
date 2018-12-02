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
#include <inttypes.h>
struct settings { //TODO avoid hardcoding palette row amount
	bool sprite;//Are we generating the palette for a sprite
	unsigned off[MAX_ROWS_PALETTE];//Offsets for each row
	unsigned alg;//Which algorithm should be used
	bool ditherAfter;//After color quantization should the image be dithered
	bool entireRow;//If true dither entire tilemap at once or false dither each row separately
	unsigned colSpace;//Which colorspace should the image be quantized in
	unsigned perRow[MAX_ROWS_PALETTE];//How many colors will be generated per row
	bool useRow[MAX_ROWS_PALETTE];
	unsigned rowAuto;
	int rowAutoEx[2];
};
void sub_tile_map(uint32_t oldTile, uint32_t newTile, bool hflip = false, bool vflip = false);
void generate_optimal_palette(Fl_Widget*, void * row);
void generate_optimal_paletteapply(Fl_Widget*, void*s);
