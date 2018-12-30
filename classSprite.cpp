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
   Copyright Sega16 (or whatever you wish to call me) (2012-2018)
*/
#include <cstdlib>
#include <cstring>
#include <stdint.h>

#include "classSprite.h"
#include "project.h"
#include "class_tiles.h"
sprite::sprite(Project*prj) {
	this->prj = prj;

	w = h = 1;
	starttile = 0;
	int fixedRow = prj->fixedSpirtePalRow();
	palrow = fixedRow >= 0 ? fixedRow : 0;
	hflip = vflip = false;
	prio = false;
	offx = offy = 0;
	loadat = 0;
}
sprite::sprite(uint32_t wi, uint32_t hi, uint32_t palrowset, uint32_t settile, bool hf, bool vf, bool pri, uint32_t la, int32_t ox, int32_t oy, Project*prj) {
	this->prj = prj;

	w = wi;
	h = hi;
	int fixedRow = prj->fixedSpirtePalRow();

	if (fixedRow >= 0)
		palrow = fixedRow;
	else
		palrow = palrowset;

	starttile = settile;
	hflip = hf;
	vflip = vf;
	prio = pri;
	loadat = la;
	offx = ox;
	offy = oy;
}
void sprite::draw(unsigned x, unsigned y, unsigned zoom) {
	unsigned yy = y;
	int32_t t = starttile;

	if (hflip)
		t += (w - 1) * h;

	for (unsigned i = 0; i < w; ++i) { //Width and height are swapped due to the way Sega genesis stores sprites. The code is the same for NES because width will always be one
		if (vflip)
			t += h - 1;

		for (unsigned j = 0; j < h; ++j) {
			prj->tileC->draw_tile(x, yy, t, zoom, palrow, hflip, vflip, true);
			yy += prj->tileC->height() * zoom;

			if (vflip)
				--t;
			else
				++t;
		}

		if (vflip)
			t += h + 1;

		if (hflip)
			t -= h * 2;

		x += prj->tileC->width() * zoom;
		yy = y;
	}
}
void sprite::toImage(uint8_t*img) {
	unsigned tileWidth = prj->tileC->width();
	unsigned tileHeight = prj->tileC->height();

	for (unsigned y = 0, ctile = starttile; y < h; ++y) {
		for (unsigned x = 0; x < w; ++x, ++ctile) {
			uint8_t*outptr = prj->tileC->truetDat.data() + (ctile * prj->tileC->tcSize);

			for (unsigned i = 0; i < 8; ++i)
				memcpy(img + (x * 8 * 4) + (y * w * 8 * 4) + (i * w * 4), outptr + (i * 8 * 4), 8 * 4);
		}
	}
}
